COBJS:=$(patsubst %.c,%.o,$(wildcard source/*.c))
CPPOBJS:=$(patsubst %.cpp,%.o,$(wildcard source/*.cpp))
OBJS:=$(COBJS) $(CPPOBJS)
ROADDETECTEXE=RoadDetect.out

CC = icpc
CFLAGS = -O2 -xHost -DLINUX -DUSBKEY -DH264_DECODE -DH264_ENCODE -DGLOBALCARCOLOR -DGLOBALCARCLASSIFY -DBELTDETECT
CFLAGS += -DGLOBALCARLABEL -DNOEVENT -DALGORITHM_DL -DLED_STATE


all: $(ROADDETECTEXE)

INC = -I../Algorithm/Surf/include -I../Algorithm/Sift/include -I../Application/Sha/include -I../Algorithm/Tsai/include
INC += -I../Algorithm/LineSeg/include -I../Application/Mjpeg2Avi/include -I../Application/H264Decode/include -I../Application/H264Encode/include -I../Application/RtspSend/include
INC += -I../Application/RtspReceive/include -I../Algorithm/VibeModel/include -I../Algorithm/Hog/include -I../Algorithm/BaseCommon/include -I../Algorithm/RedAdjust/include
INC += -I../Algorithm/ImageEnhance/include -I../Algorithm/CTinyXml/include -I../Algorithm/VideoStab/include
INC += -I../Algorithm/TruckDetect/include -I../Algorithm/NewColorReg/include -I../Algorithm/CarLabelDL/includeDL -I../Algorithm/CameraControl/include 
INC += -I../Algorithm/VideoShut/include -I../Algorithm/CWindShield/include -I../Algorithm/FindTarget/include -I../Algorithm/NewTrack/include -I../Algorithm/SeatBelt/include
INC += -I/usr/local/opencv-2.3.1/include -I/usr/local/opencv-2.3.1/include/opencv -I/detectdata/opt/ipp8.2/ipp/include -I../Application/Cximage/CxImage -Iinclude
INC += -I/usr/include/freetype2 -I./Driver/v4l2 -I./Driver/v4l2_capture
INC += -I../Application/Livkey/include -I../Algorithm/PeoCarDetect/include -I../Algorithm/NoPlateVehicle/include
INC += -I../Algorithm/CnnBaseLib/include -I../Algorithm/LicensePlateLoc/include -I../Algorithm/MvPLR_New_Lib/include -I../Algorithm/PlateCharLocation/include
INC += -I../Application/HttpUpload/include -I../Application/BXSdk/

LIB = -L./lib -lLivKey -lclave2 -lH264Encode -lx264 -lH264Decode -lSeatBelt -lTruckDetect -lNewColorReg
LIB += -L./lib -lCarLabelDL -lCompressModel -lCameraControl -lMvPLR_New_Lib -lLicensePlateLoc 
LIB += -L./lib -lNoPlateVehicle -lPeoCarDetect -lVideoShut -lCWindShield -lFindTarget -lNewTrack -lCxImage -lV4l2
LIB += -L./lib -lCapture -ldl -lm -pthread -lmysqlclient_r -lSurf -lSift -lSha -lTsai -lV4l2 -lCapture -lLineSeg
LIB += -L./lib -lMjpeg2Avi -lH264Encode -lRtspSend -lRtspReceive -lVibeModel -lHog -lBaseCommon -lRedAdjust
LIB += -L./lib -lImageEnhance -lCTinyXml -lLsd -lHeadShoulder -lVideoStab -ldl -lm -pthread -lmysqlclient_r
LIB += -L./lib -lejhttpupload -lPlateCharLocation_Lib -lCnnBaseLib -lBX_SDK

LIB += -L/usr/local/opencv-2.3.1/lib -lopencv_gpu -lopencv_legacy -lopencv_video -lopencv_objdetect -lopencv_contrib
LIB += -L/usr/local/opencv-2.3.1/lib -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_ml
LIB += -L/detectdata/opt/ipp8.2/composer_xe_2015/ipp/lib/intel64 -lipps -lippi -lippj -lippcc -lippcore -lippm -lippcv -lippvc
LIB += -L/usr/lib/x86_64-linux-gnu -lavutil -lavdevice -lavformat -lavcodec -lpostproc -lavfilter -lswscale -lgpac

$(ROADDETECTEXE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@  $(LIB) $(INC)
	cp -f $(ROADDETECTEXE) ../Bin/

$(CPPOBJS): %.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

clean:
		rm -rf $(OBJS) $(ROADDETECTEXE)


alllib:
	rm -rf ../sift/source/*.o
	make -C ../sift
	rm -rf ../medianbg/source/*.o
	make -C ../medianbg
	rm -rf ../lineseg/source/*.o
	make -C ../lineseg
	rm -rf ../tsai/source/*.o
	make -C ../tsai
	rm -rf ../stabilizelib/source/*.o
	make -C ../stabilizelib
