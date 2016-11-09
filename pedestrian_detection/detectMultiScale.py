# import the necessary packages
from __future__ import print_function
from imutils.object_detection import non_max_suppression
from imutils import paths
import numpy as np
import argparse
import datetime
import imutils
import cv2
 
 # creamos el analizador de parámetros y los parseamos
ap = argparse.ArgumentParser()
ap.add_argument("-i", "--image", required=True, help="path to the input image")
ap.add_argument("-w", "--win-stride", type=str, default="(8, 8)", help="window stride")
ap.add_argument("-p", "--padding", type=str, default="(16, 16)", help="object padding")
ap.add_argument("-s", "--scale", type=float, default=1.05, help="image pyramid scale")
ap.add_argument("-m", "--mean-shift", type=int, default=-1, help="whether or not mean shift grouping should be used")
args = vars(ap.parse_args())
winStride = eval(args["win_stride"])
padding = eval(args["padding"])
meanShift = True if args["mean_shift"] > 0 else False
# inicializamos el descriptor el HOG descriptor/person detector
hog = cv2.HOGDescriptor()
hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
# cargamos la imagen y cambiamos su tamaño para:
# 1 -> Reducir el tiempo de detección
# 2 -> mejorar la precisión en la detección
image = cv2.imread(args["image"])
image = imutils.resize(image, width=min(400, image.shape[1]))
orig = image.copy()
# detectamos personas en la imagen
start = datetime.datetime.now()
(rects, weights) = hog.detectMultiScale(image, winStride=winStride, 
    padding=padding, scale=args["scale"], useMeanshiftGrouping=meanShift)
print("[INFO] detection took: {}s".format((datetime.datetime.now() - start).total_seconds()))
(rects, weights) = hog.detectMultiScale(image, winStride=(4, 4), padding=(8, 8), scale=1.05)
# apply NMS algorithm para reducir el número de falsos positivos reportados por el detector
rects = np.array([[x, y, x + w, y + h] for (x, y, w, h) in rects])
pick = non_max_suppression(rects, probs=None, overlapThresh=0.65)
# dibujamos los contornos finales
for (xA, yA, xB, yB) in pick:
    cv2.rectangle(image, (xA, yA), (xB, yB), (0, 255, 0), 2)
print("[INFO] {} original boxes, {} after suppression".format(len(rects), len(pick)))
# Mostramos imagenes
cv2.imshow("Before NMS", orig)
cv2.imshow("After NMS", image)
cv2.waitKey(0)
 
 