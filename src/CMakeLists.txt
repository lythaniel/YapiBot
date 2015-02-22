cmake_minimum_required(VERSION 2.8)
project( PoBot )
SET(COMPILE_DEFINITIONS -Werror)
find_package( OpenCV REQUIRED )
find_path (OPT_VC bcm_host.h /opt/vc/include/)
find_path (RPI_GPIO pigpio.h /usr/local/include)
find_path (RPI_SOCKET socket.h /usr/include/linux)
find_path (LIB_GPIO_PATH libpigpio.a /usr/local/lib/)
find_path (RPI_LIBS libpthread_nonshared.a /usr/lib/arm-linux-gnueabihf/)
find_PATH (RPI_SYS_LIBS libpthread.so.0 /lib/arm-linux-gnueabihf/)
include_directories(${RPI_GPIO})
include_directories(${OPT_VC})
include_directories(${OPT_VC}/../userland/interface/vcos)
include_directories(${OPT_VC}/../userland)
include_directories(${OPT_VC}/../userland/interface/vcos/pthreads)
include_directories(${OPT_VC}/../userland/interface/vmcs_host/linux)
include_directories(${OPT_VC}/../userland/host_applications/linux/libs/bcm_host/include)
link_directories(${OPT_VC}/../lib/)
link_directories(${OPT_VC}/../userland/host_applications/linux/libs/)
link_directories(${OPT_VC}/../userland/build/lib/)
link_directories(${LIB_GPIO_PATH})
link_directories(${RPI_SYS_LIBS})
link_directories(${RPI_LIBS})


add_executable(
	PoBot 
	PoBot.cpp
	Camera.cpp
	CameraControl.cpp
	Graphics.cpp
	Motors.cpp
	Encoder.cpp
	Thread.cpp
	Network.cpp
	Mutex.cpp
	Semaphore.cpp
	Controller.cpp
	ImageProcessing.cpp
	VideoStreamer.cpp
	Compass.cpp
	I2Cbus.cpp
	Sampler.cpp
	RangeFinder.cpp
	EventObserver.cpp
)

target_link_libraries(
	PoBot
	libmmal_core.so
	libmmal_util.so
	libmmal_vc_client.so
	libvcos.so
	libbcm_host.so
	GLESv2
	EGL
	${OpenCV_LIBS}
	pigpio
	pthread
	rt
)

