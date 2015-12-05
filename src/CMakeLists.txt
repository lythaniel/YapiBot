cmake_minimum_required(VERSION 2.8)
project( PoBot )
SET(COMPILE_DEFINITIONS -Werror -pthread)
find_package( OpenCV REQUIRED )
find_package( PythonLibs REQUIRED)
#set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package( Threads REQUIRED )
#target_compile_options(PUBLIC PoBot "-pthread")
SET(CMAKE_CXX_FLAGS_DEBUG "-lpthread -g -o0 ") 
SET(CMAKE_CXX_FLAGS_RELEASE "-lpthread -o3")

find_path (OPT_VC bcm_host.h /opt/vc/include/)
find_path (RPI_GPIO pigpio.h /usr/local/include)
find_path (RPI_SOCKET socket.h /usr/include/linux)
find_path (LIB_GPIO_PATH libpigpio.so /usr/local/lib/)
find_path (RPI_LIBS libpthread_nonshared.a /usr/lib/arm-linux-gnueabihf/)
find_PATH (RPI_SYS_LIBS libpthread.so.0 /lib/arm-linux-gnueabihf/)
find_PATH (GNU_EABIHF_PATH fpu_control.h /usr/include/arm-linux-gnueabihf/)

include_directories(${RPI_GPIO})
include_directories(${OPT_VC})
#include_directories(${OPT_VC}/../userland/interface/vcos)
#include_directories(${OPT_VC}/../userland)
#include_directories(${OPT_VC}/../userland/interface/vcos/pthreads)
#include_directories(${OPT_VC}/../userland/interface/vmcs_host/linux)
#include_directories(${OPT_VC}/../userland/host_applications/linux/libs/bcm_host/include)
include_directories(${OPT_VC}/interface/vcos)
include_directories(${OPT_VC}/)
include_directories(${OPT_VC}/interface/vcos/pthreads)
include_directories(${OPT_VC}/interface/vmcs_host/linux)
include_directories(${OPT_VC}/host_applications/linux/libs/bcm_host/include)

include_directories(${GNU_EABIHF_PATH})

include_directories(${PYTHON_INCLUDE_DIRS}/)


find_PATH (USR_LIBS libgtkglext-x11-1.0.so.0 /usr/lib/)

find_library (GPIO_LIBS libpigpio.so /usr/local/lib/)
#find_library (PTHREAD_LIBS libpthread.so.0 /lib/arm-linux-gnueabihf/) 
find_library (GLIB_LIBS libglib-2.0.so.0 /lib/arm-linux-gnueabihf/) 
find_library (M_LIBS libm.so.6 /lib/arm-linux-gnueabihf/) 
find_library (LD_LIBS ld-linux-armhf.so.3 /lib/arm-linux-gnueabihf/) 
find_library (C_LIBS libc.so.6 /lib/arm-linux-gnueabihf/) 
find_library (PCRE_LIBS libpcre.so.3 /lib/arm-linux-gnueabihf/) 
find_library (STD_LIBS libstdc++.so.6 /lib/arm-linux-gnueabihf/) 




Message (STATUS "RPI system lib directory:" ${RPI_SYS_LIBS})
Message (STATUS "RPI lib directory:" ${RPI_LIBS})
Message (STATUS "RPI opt_vc lib directory:" ${OPT_VC})
Message (STATUS "pthread libs:" ${PTHREAD_LIBS})
Message (STATUS "usr libs:" ${USR_LIBS})
Message (STATUS "gpio libs:" ${GPIO_LIBS})
Message (STATUS "glibs:" ${GLIB_LIBS})
Message (STATUS "LD_LIBRARY_PATH:" ${LD_LIBRARY_PATH})

link_directories(${USR_LIBS}/)
link_directories(${OPT_VC}/../lib/)

#link_directories(${OPT_VC}/../userland/host_applications/linux/libs/)
#link_directories(${OPT_VC}/../userland/build/lib/)
#link_directories(${LIB_GPIO_PATH}/)
link_directories(${RPI_SYS_LIBS}/)
#link_directories(${RPI_LIBS}/)

#find_package(PkgConfig)
#pkg_check_modules(GLIB REQUIRED glib-2.0)
#include_directories(${GLIB_INCLUDE_DIRS})

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
	ScriptEngine.cpp
	PythonApi.cpp
	PythonBinding.cpp
)

target_link_libraries(
	PoBot
	${C_LIBS}
	${GLIB_LIBS}
rt
	${M_LIBS}
	${LD_LIBS}
	${PCRE_LIBS}
	${STD_LIBS}
	libmmal_core.so
	libmmal_util.so
	libmmal_vc_client.so
	libvcos.so
	libbcm_host.so
	GLESv2
	EGL
	${PYTHON_LIBRARIES}
	${OpenCV_LIBS}
	${GPIO_LIBS}

	
	pthread
	

)



