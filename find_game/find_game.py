import numpy as np
import cv2

# cargamos la imagen de los tres juegos
image = cv2.imread("games.jpg")

# creamos una máscara destacando en blanco las zonas de color rojo y el resto en negro
lower = np.array([0, 0, 200])
upper = np.array([65, 65, 255])
mask = cv2.inRange(image, lower, upper)

# buscamos los contornos en la imagen enmascarada y nos quedamos con el más grande
(_, cnts, _) = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
c = max(cnts, key=cv2.contourArea)

# aproximamos el contorno
peri = cv2.arcLength(c, True)
approx = cv2.approxPolyDP(c, 0.05 * peri, True)

# dibujamos un contorno de color verde alrededor de la imagen
cv2.drawContours(image, [approx], -1, (0, 255, 0), 4)
cv2.imshow("Image", image)
cv2.waitKey(0)