frame_nums = [617,1151,11,1169,1504,930,215,1795,337,1775] #случайные кадры
src_f = 'old_commit_frames'
dest_f = 'last_commit_frames'
import imutils
import cv2
from skimage.metrics import structural_similarity as compare_ssim
import numpy as np

for frame in frame_nums:
    old, new = f"{src_f}/frame-{frame}.png", f"{dest_f}/frame-{frame}.png"
    i_old, i_new = cv2.imread(old), cv2.imread(new)
    gray_old, gray_new = cv2.cvtColor(i_old, cv2.COLOR_BGR2GRAY), cv2.cvtColor(i_new, cv2.COLOR_BGR2GRAY)
    # print(type(gray_old),gray_old.shape)
    #gray_... shape = [height, width]
    for row in range(0, gray_old.shape[0], 16):
        for col in range(0, gray_old.shape[1], 16):
            old_block = gray_old[row:row+16, col:col+16]
            new_block = gray_new[row:row+16, col:col+16]
            old_mean, new_mean = old_block.mean(), new_block.mean()
            gray_old[row:row+16, col:col+16] = old_mean
            gray_new[row:row+16, col:col+16] = new_mean


    # (_, diff) = compare_ssim(gray_old, gray_new, full=True)
    diff = cv2.subtract(gray_old, gray_new)
    # diff = (diff * 255).astype("uint8")
    # thresh = cv2.threshold(diff, 0, 255,
	#     cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)[1]
    # cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
    #     cv2.CHAIN_APPROX_SIMPLE)
    # cnts = imutils.grab_contours(cnts)
    # for c in cnts:
    #     # compute the bounding box of the contour and then draw the
    #     # bounding box on both input images to represent where the two
    #     # images differ
    #     (x, y, w, h) = cv2.boundingRect(c)
    #     cv2.rectangle(i_old, (x, y), (x + w, y + h), (0, 0, 255), 2)
    #     cv2.rectangle(i_new, (x, y), (x + w, y + h), (0, 0, 255), 2)
    # show the output images
    cv2.imshow("default", i_old)
    cv2.imshow("Modified", i_new)
    cv2.imshow("default scaled", gray_old)
    cv2.imshow("Modified scaled", gray_new)
    cv2.imshow("Diff", diff)
    # cv2.imshow("Thresh", thresh)
    cv2.waitKey(0)
    