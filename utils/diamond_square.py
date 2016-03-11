#!/usr/bin/python3

import argparse
import math
import random

import numpy
from PIL import Image

def scaled_random(mag):
    return (2 * random.random() - 1.0) * mag

def diamond(array, x, y, size, mag):
    """For each square in the array, set the midpoint of that square to be the
    average of the four corner points plus a random value.
    """
    half = size // 2

    x1 = x + size
    y1 = y + size

    a = array[x, y]
    b = array[x1, y]
    c = array[x1, y1]
    d = array[x, y1]

    array[x + half, y + half] = (a + b + c + d) / 4.0 + scaled_random(mag)

def square(array, x, y, size, mag):
    """For each diamond in the array, set the midpoint of that diamond to be
    the average of the four corner points plus a random value.  """

    x1 = x - size
    y1 = y - size
    x2 = x + size
    y2 = y + size

    div = 4.0

    l = len(array)

    if x1 >= 0:
        a = array[x1, y]
    else:
        a = 0.0
        div -= 1.0
    if y1 >= 0:
        b = array[x, y1]
    else:
        b = 0.0
        div -= 1.0
    if x2 < l:
        c = array[x2, y]
    else:
        c = 0.0
        div -= 1.0
    if y2 < l:
        d = array[x, y2]
    else:
        d = 0.0
        div -= 1.0

    if div:
        array[x, y] = (a + b + c + d) / div + scaled_random(mag)

def diamond_square(array, step, mag, red):
    if step < 2:
        return

    l = len(array) - 1
    half = step // 2

    for x in range(0, l, step):
        for y in range(0, l, step):
            diamond(array, x, y, step, mag)

    for x in range(0, l + 1, step):
        for y in range(0, l + 1, step):
            if x < l:
                square(array, x + half, y, half, mag)
            if y < l:
                square(array, x, y + half, half, mag)

    mag *= red

    diamond_square(array, half, mag, red)

def main():
    parser = argparse.ArgumentParser("Diamond-square terrain generator")
    parser.add_argument("--seed", type=int, default=None,
                        help="Random seed")
    parser.add_argument("--size", type=int, default=256,
                        help="Image size (rounded up to power of 2)")
    parser.add_argument("filename", nargs=1,
                        help="Target file name")
    args = parser.parse_args()

    if args.size < 2:
        args.size = 2
    else:
        args.size = 2 ** int(math.log2(args.size - 1) + 1)

    if args.seed:
        random.seed(args.seed)
    else:
        random.seed()

    array = numpy.zeros((args.size + 1, args.size + 1), dtype=numpy.float32)

    array[0,0] = scaled_random(10)
    array[0,args.size] = scaled_random(1)
    array[args.size,0] = scaled_random(1)
    array[args.size,args.size] = scaled_random(1)

    diamond_square(array, args.size, 10.0, 0.5)

    minimum = numpy.amin(array)
    maximum = numpy.amax(array)

    array -= minimum
    maximum -= minimum

    array = array * (255.0 / maximum)

    barray = numpy.array(array, dtype=numpy.ubyte)

    im = Image.frombuffer("L", (args.size + 1, args.size + 1),
                        memoryview(barray), "raw", "L", 0, 1)
    im = im.crop((0, 0, args.size, args.size))

    px = im.load()

    im.save(args.filename[0])

if __name__ == "__main__":
    main()
