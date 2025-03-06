d_ffmpeg = /home/ressiwage/default-ffmpeg/usr/bin/ffmpeg
d_ffmpeg = ffmpeg

perf_path = /usr/lib/linux-tools/5.15.0-133-generic/perf

usage:
	echo "make fetch_small_bunny_video && make run_hello"

all: clean fetch_bbb_video make_hello run_hello make_remuxing run_remuxing_ts run_remuxing_fragmented_mp4 make_transcoding
.PHONY: all

clean:
	@rm -rf ./build/*
clear_temp:
	rm temp/frame-* ; rm temp/*.png ; rm temp/dc_frame*.png ; rm temp/dc_*.pgm

fetch_small_bunny_video:
	./fetch_bbb_video.sh

make_hello: clean 
	gcc -g -L/usr/local/lib  -I/home/ressiwage/projects/FFmpeg-nvidia-build/ffmpeg  0_hello_world.c \
		-lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpng \
		-o ./build/hello

make_hello_prof: clean 
	gcc -g -pg -L/usr/local/lib -I/home/ressiwage/projects/FFmpeg-nvidia-build/ffmpeg 0_hello_world.c \
		-lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil \
		-o ./build/hello

scratch:
	gcc scratch.c -o testing && ./testing

run_hello_short: make_hello
	./build/hello /home/ressiwage/projects/test-libav/test-decoding/small_bunny_1080p_60fps.mp4

hello_to_movie:
	ffmpeg -y -framerate 30 -i temp/frame-%d.pgm -vf "pad=width=ceil(iw/2)*2:height=ceil(ih/2)*2" -c:v libx264 -pix_fmt yuv420p output_hi.mp4 ; make clear_temp; explorer.exe output_hi.mp4

hello_to_heatmap:
	ffmpeg -y -framerate 30 -i temp/frame-%d.pgm -vf "scale=144:-1" -c:v libx264 -pix_fmt yuv420p output_hi_heatmap.mp4

#cavlc
T_T_S = /home/ressiwage/projects/test-libav/test-decoding/small-bunny-lowres.mp4
# T_T_S = /home/ressiwage/projects/test-libav/test-decoding/small_bunny_1080p_60fps.mp4
#  T_T_S = /home/ressiwage/projects/frames-decoding/b264t.mkv
T_T_S = /home/ressiwage/projects/frames-decoding/b264t_half.mp4

#cabac
# T_T_S=/home/ressiwage/projects/test-libav/test-decoding/b264t_cabac.mkv
# T_T_S=/home/ressiwage/projects/test-libav/test-decoding/b264t_half_cabac.mkv
# T_T_S=/home/ressiwage/projects/test-libav/test-decoding/b264t_small_cabac.mkv
# T_T_S = '/mnt/d/torrent/Dog Day Afternoon (1975) BDRip.mkv'

#265
# T_T_S = /home/ressiwage/projects/test-libav/test-decoding/b265t.mp4 # this is cabaced b264
# T_T_S = /home/ressiwage/projects/test-libav/test-decoding/b265t_half.mp4 # this is cavlced b264_half
# T_T_S = /home/ressiwage/projects/test-libav/test-decoding/b265t_small.mp4 #cabaced and truncated

#AV1
T_T_S = /home/ressiwage/projects/test-libav/test-decoding/bAV1t_half.mp4
# T_T_S = /home/ressiwage/projects/test-libav/test-decoding/bAV1t_small.mp4
T_T_S = /home/ressiwage/projects/test-libav/test-decoding/bAV1t_action.mp4


run_hello: make_hello 
	clear_temp;./build/hello $(T_T_S)

time_hello: make_hello
	clear_temp; time ./build/hello $(T_T_S)

run_test: make_hello
	cmdbench --iterations 3 --print-averages  --save-json bench.json --save-plot=plot.png "./build/hello $(T_T_S)" && cd temp && python3 ../pgms_to_pngs.py && cd ..

run_prof: make_hello_prof
	./build/hello $(T_T_S)

show_prof: run_prof
	gprof ./build/hello   

run_test_short: make_hello
	cmdbench --iterations 2 --print-averages --print-values --save-json bench.json --save-plot=plot.png "./build/hello  $(T_T_S)" && \
	cd temp && python3 ../pgms_to_pngs.py && cd .. 

trace_test_short: make_hello
	ltrace -c -x "@*aom*" -x "@libavcodec.so*" -x "@libavformat.so*" -S -f ./build/hello  $(T_T_S)

pgm_to_images:
	cd temp && python3 ../pgms_to_pngs.py && cd ..

images_to_video:
	/bin/python3 /home/ressiwage/projects/test-libav/test-decoding/pngs_to_video.py



trace_bench: run_test_short trace_test_short

build_macro: clean
	gcc -g -L/usr/local/lib -I/home/ressiwage/projects/FFmpeg-nvidia-build/ffmpeg  1_extract_macroblocks.c \
		-lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil \
		-o ./build/macro

run_macro: build_macro
	./build/macro $(T_T_S)

bench_macro: build_macro
	cmdbench "./build/macro $(T_T_S)"

trace_macro: build_macro
	ltrace -c -S ./build/macro $(T_T_S)

build_me: clean
	gcc -g -L/opt/ffmpeg/lib -I/home/ressiwage/projects/FFmpeg-nvidia-build/ffmpeg  4_maxim_example.c -lavcodec -lavformat -lavutil -o build/extract_dc   

run_me: build_me
	./build/extract_dc $(T_T_S)

build_me_f: clean 
	make clear_temp; gcc -g -L/usr/local/lib -I/home/ressiwage/projects/FFmpeg-nvidia-build/ffmpeg  5_maxim_ex_fixed.c -lavcodec -lavformat -lavutil -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -o build/extract_dc_f

run_me_f: build_me_f
	./build/extract_dc_f $(T_T_S)

pipeline_f: run_me_f 
	mv output_dc.mp4 output_dc_prev.mp4; ffmpeg -y -framerate 30 -i temp/dc_frame_%d.pgm -vf "pad=width=ceil(iw/2)*2:height=ceil(ih/2)*2" -c:v libx264 -pix_fmt yuv420p output_dc.mp4
	make clear_temp

# requires proper ffmpeg build with minimum modifications plus wsl gui apps enabled
# how to launch: make compare M1=movie1 M2 = movie2
M1 = etalon.mp4
M2 = output_hi.mp4
compare:
	ffplay -f lavfi \
	"movie=$(M1)[org]; \
 	movie=$(M2)[enc]; \
 	[org][enc]blend=all_mode=difference"


compare_heatmaps: run_hello, hello_to_heatmap
	compare M1=output_hi_heatmap.mp4 M2=etalon_heatmap.mp4


profile_and_show_in_chrome: make_hello_prof
	uftrace record --nest-libcall ./build/hello  $(T_T_S) && \
	uftrace dump --chrome > uftrace-dump-chrome.json &&\
	python3 /home/ressiwage/projects/catapult/tracing/bin/trace2html ./uftrace-dump-chrome.json

mytrace:
	time ( make run_hello | python3 timing.py )

cachegrind:
	valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes ./build/hello $(T_T_S)