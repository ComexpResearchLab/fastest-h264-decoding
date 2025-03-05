```c

void av1_idct8(const int32_t *input, int32_t *output, int8_t cos_bit,
               const int8_t *stage_range) {
  assert(output != input);
  const int32_t size = 8;
  const int32_t *cospi = cospi_arr(cos_bit);

  int32_t stage = 0;
  int32_t *bf0, *bf1;
  int32_t step[8];

  // stage 0;

  // stage 1;
  /*
  scan8 = [0, 4, 2, 6,  1, 5, 3, 7]
  reorder: output[i] = input[scan8[i]]
  */
  stage++;
  output[0] = input[0]; // 0=0
  output[1] = input[4]; //1=4
  output[2] = input[2]; // 2=2
  output[3] = input[6]; //3=6
  output[4] = input[1]; //4=1
  output[5] = input[5]; // 5=5
  output[6] = input[3]; //6=3
  output[7] = input[7]; // 7=7
  av1_range_check_buf(stage, input, bf1, size, stage_range[stage]);

  // stage 2
  /*
  step[:4] = reordered input[:4]
  step[4:] = avg(i, элемент зеркальный i по оси 5.5)
  */
  stage++;
  step[0] = input[0];
  step[1] = input[4];
  step[2] = input[2];
  step[3] = input[6];
  step[4] = half_btf(cospi[56], input[1], -cospi[8],  input[7], cos_bit);
  step[5] = half_btf(cospi[24], input[5], -cospi[40], input[3], cos_bit);
  step[6] = half_btf(cospi[40], input[5], cospi[24],  input[3], cos_bit);
  step[7] = half_btf(cospi[8],  input[1], cospi[56],  input[7], cos_bit);
  av1_range_check_buf(stage, input, bf1, size, stage_range[stage]);

  // stage 3
  /*
  output[:4] = 
  */
  stage++;
  bf0 = step;
  bf1 = output;
  output[0] = half_btf(cospi[32], input[0], cospi[32],  input[4], cos_bit);
  output[1] = half_btf(cospi[32], input[0], -cospi[32], input[4], cos_bit);
  output[2] = half_btf(cospi[48], input[2], -cospi[16], input[6], cos_bit);
  output[3] = half_btf(cospi[16], input[2], cospi[48],  input[6], cos_bit);
  output[4] = clamp_value( f(input[1], input[7])  + f(input[5], input[3]), stage_range[stage]);
  output[5] = clamp_value( f(input[1], input[7])  - f(input[5], input[3]), stage_range[stage]);
  output[6] = clamp_value( f(input[5], input[3])  + f(input[1], input[7]), stage_range[stage]);
  output[7] = clamp_value( f(input[5], input[3])  + f(input[1], input[7]), stage_range[stage]);
  av1_range_check_buf(stage, input, bf1, size, stage_range[stage]);

  // stage 4
  stage++;
  bf0 = output;
  bf1 = step;
  step[0] = clamp_value(output[0] + output[3], stage_range[stage]);
  step[1] = clamp_value(output[1] + output[2], stage_range[stage]);
  step[2] = clamp_value(output[1] - output[2], stage_range[stage]);
  step[3] = clamp_value(output[0] - output[3], stage_range[stage]);
  step[4] = output[4];
  step[5] = half_btf(-cospi[32], output[5], cospi[32], output[6], cos_bit);
  step[6] = half_btf(cospi[32], output[5], cospi[32], output[6], cos_bit);
  step[7] = output[7];
  av1_range_check_buf(stage, input, bf1, size, stage_range[stage]);

  // stage 5
  stage++;
  bf0 = step;
  bf1 = output;
  output[0] = clamp_value(step[0] + step[7], stage_range[stage]);
  output[1] = clamp_value(step[1] + step[6], stage_range[stage]);
  output[2] = clamp_value(step[2] + step[5], stage_range[stage]);
  output[3] = clamp_value(step[3] + step[4], stage_range[stage]);
  output[4] = clamp_value(step[3] - step[4], stage_range[stage]);
  output[5] = clamp_value(step[2] - step[5], stage_range[stage]);
  output[6] = clamp_value(step[1] - step[6], stage_range[stage]);
  output[7] = clamp_value(step[0] - step[7], stage_range[stage]);
}

```