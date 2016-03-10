#!/usr/bin/python3

import argparse
import math
import random

from PIL import Image


def main():
    parser = argparse.ArgumentParser("Island mask generator")
    parser.add_argument("--width", type=int, default=256,
                        help="Image width")
    parser.add_argument("--height", type=int, default=256,
                        help="Image height")
    parser.add_argument("filename", nargs=1,
                        help="Target file name")
    args = parser.parse_args()

    im = Image.new("L", (args.width, args.height))

    px = im.load()

    random.seed()

    # build Fourier series for the island envelopes (outer and inner)
    f = {}
    m = {}
    for i in range(2):
        print("series %i:" % (i + 1),)
        f[i] = { 0: (0, 0) }
        m[i] = 0
        k = 1
        for j in range(1, 6):
            a = k * (2 * random.random() - 1.0)
            d = math.pi * (2 * random.random() - 1.0)
            f[i][j] = (a, d)
            print("A%i=%6.2f p%i=%6.2f" % (j, a, j, d))
            k = 0.9 * k
            m[i] += abs(a)

    mid_x = im.width / 2
    mid_y = im.height / 2
    r_max = min(mid_x, mid_y)

    r_band = r_max / 6

    r_out = r_max - r_band
    r_in = r_out - 2 * r_band

    for x in range(0, im.width):
        for y in range(0, im.height):
            rx = x - mid_x
            ry = y - mid_y
            r = math.sqrt(rx ** 2 + ry ** 2)
            phi = math.atan2(ry, rx)

            k = [f[0][0][0], f[1][0][0]]
            for i in range(2):
                for j in range(1, 6):
                    k[i] += f[i][j][0] * math.sin(j * phi + f[i][j][1])

            r1 = r_in + r_band * k[0] / m[0]
            r2 = r_out + r_band * k[1] / m[1]

            if r < r1 or r2 == r1:
                v = 255
            elif r > r2:
                v = 0
            else:
                v = 1 - (r - r1) / (r2 - r1)
            px[x, y] = int(255 * v)

    im.save(args.filename[0])


if __name__ == "__main__":
    main()
