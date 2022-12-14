import imageio
import numpy as np
import os
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity as ssim
from skimage.metrics import mean_squared_error as mse


def psnr_wrapper(img_true, img_test):
    return psnr(img_true, img_test, data_range=10.0)


def ssim_wrapper(img1, img2):
    return ssim(img1, img2)


def rmse_wrapper(img1, img2):
    return np.sqrt(mse(img1, img2))

model = ["1108001", "1209001", "1209002"]

for name in model:

    im_dir = f"/home/lzr/Projects/mitsuba/experiment/ablation/{name}/sphere/"
    gt_dir = "/home/lzr/Projects/mitsuba/experiment/ablation/gy/sphere/"

    index = []
    with open('/home/lzr/Projects/mitsuba/experiment/ablation/material_set.txt', 'r') as f:
        while True:
            line = f.readline()[:-1]
            if not line:
                break
            im = np.array(imageio.imread(os.path.join(im_dir,line,line+'_twolobe.exr')), dtype=np.float32)
            gt = np.array(imageio.imread(os.path.join(gt_dir,line,line+'_gy.exr')), dtype=np.float32)
            index.append([line, rmse_wrapper(gt, im)])
            
    with open(os.path.join(im_dir, f'index_rmse_{name}.csv'), 'w') as outfile:
        for pair in index:
            outfile.write(f'{pair[0]},{pair[1]:.8f}')
            outfile.write('\n')
