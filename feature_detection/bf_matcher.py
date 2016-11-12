import numpy as np
import cv2
from matplotlib import pyplot as plt

img1 = cv2.imread('img/2093GSW.jpg') # queryImage
img1Gray = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)
img2 = cv2.imread('img/matricula-zoom.jpg') # trainImage
img2Gray = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)

# Initiate SURF detector
surf = cv2.xfeatures2d.SURF_create()

# find the keypoints and descriptors with SURF
(kp1, des1) = surf.detectAndCompute(img1Gray,None)
print("# kp1: {}, descriptors1: {}".format(len(kp1), des1.shape))
(kp2, des2) = surf.detectAndCompute(img2Gray,None)
print("# kp2: {}, descriptors2: {}".format(len(kp2), des2.shape))
# BFMatcher with default params
bf = cv2.BFMatcher()
matches = bf.knnMatch(des1,des2, k=2)

# Apply ratio test
good = []
for m,n in matches:
    if m.distance < 0.75*n.distance:
        good.append([m])

# cv2.drawMatchesKnn expects list of lists as matches.
img3 = cv2.drawMatchesKnn(img1,kp1,img2,kp2,matches,None,flags=2)

plt.imshow(img3),plt.show()