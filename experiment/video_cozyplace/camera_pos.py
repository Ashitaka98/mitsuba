import numpy as np
from scipy.spatial.transform import Rotation as R

lookat = np.array([0.027, 0.4, -2.603])
origin = np.array([0.027, 0.4, 0])
v = origin - lookat
with open(
        '/home/lzr/Projects/mitsuba/experiment/video_cozyplace/camera_pos.txt',
        'w') as f:
    for deg in range(-30, 31, 1):
        r = R.from_euler('y', deg, degrees=True)
        pos = r.apply(v) + lookat
        f.write(str(pos[0]) + '_' + str(pos[1]) + '_' + str(pos[2]) + '\n')
