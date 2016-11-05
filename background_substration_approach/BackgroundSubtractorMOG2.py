import argparse
import datetime
import imutils
import time
import cv2
 
# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video", help="path to the video file")
ap.add_argument("-a", "--min-area", type=int, default=500, help="minimum area size")
args = vars(ap.parse_args())
 
# if the video argument is None, then we are reading from webcam
if args.get("video", None) is None:
	camera = cv2.VideoCapture(0)
	time.sleep(0.25)
 
# otherwise, we are reading from a video file
else:
	camera = cv2.VideoCapture(args["video"])
 
fgbg = cv2.createBackgroundSubtractorMOG2(detectShadows = True)

# loop over the frames of the video
while True:
    # Obtenemos el frame actual e inicializamos el texto de ocupado a no ocupado
    # grabbed -> indica si el frame ha sido leido exitosamente desde el buffer.
    (grabbed, frame) = camera.read()
    text = "Unoccupied"
    #Si el frame no puede ser grabado, entonces finalizamos la grabación
    if not grabbed:
        break
    frameDelta = fgbg.apply(frame)
    # Aplicamos un umbral para detectar las regiones con cambios significativos.
    # Si el píxel delta es menor que 25 lo descartmos (se queda negro), si supera el lo umbral lo consideramos blanco.
    thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
    # destacamos el primer plano y obtenemos el contorno
    thresh = cv2.dilate(thresh, None, iterations=2)
    (_, cnts, _) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    # iteramos sobre los contornos
    for c in cnts:
        # Si el contorno es demasido pequeño lo descartamos
        if cv2.contourArea(c) < args["min_area"]:
            continue
        # calculamos el cuadro de límite para el contorno, lo dibujamos en el marco, y actualizamos el texto
        (x, y, w, h) = cv2.boundingRect(c)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        text = "Occupied"
    # dibujamos el texto y la marca de tiempo del frame
    cv2.putText(frame, "Room Status: {}".format(text), (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
    cv2.putText(frame, datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"),
		(10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)

    cv2.imshow("Security Feed", frame)
    cv2.imshow("Thresh", thresh)
    key = cv2.waitKey(1) & 0xFF
    # Si pulsa la tecla 'q', salimos de la grabación
    if key == ord("q"):
        break
 
# libera la cámara y eliminamos todas las ventanas
camera.release()
cv2.destroyAllWindows()

img = cv2.imread('street.jpg')

fgmask = fgbg.apply(img)
cv2.imshow('frame',img)
cv2.imshow('frame mask',fgmask)
cv2.waitKey(0)