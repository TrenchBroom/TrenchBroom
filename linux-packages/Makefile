DOCKER_BASE64="ubuntu:14.04"
DOCKER_BASE32="ioft/i386-ubuntu:14.04"

dirs:
	test -d build32-out || mkdir build32-out
	test -d build64-out || mkdir build64-out
	test -d pkg-out || mkdir pkg-out

image-64:
	echo "FROM $(DOCKER_BASE64)" | cat - base.Dockerfile | docker build -t trenchbroom/build64 -

build-64: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build64-out:/build -v $(PWD)/script:/script trenchbroom/build64 /script/build.sh

package-64: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build64-out:/build -v $(PWD)/script:/script trenchbroom/build64 /script/package.sh

bash-64: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build64-out:/build -v $(PWD)/script:/script -i -t trenchbroom/build64 /bin/bash

image-32:
	echo "FROM $(DOCKER_BASE32)" | cat - base.Dockerfile | docker build -t trenchbroom/build32 -

build-32: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build32-out:/build -v $(PWD)/script:/script trenchbroom/build32 /script/build.sh

package-32: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build32-out:/build -v $(PWD)/script:/script trenchbroom/build32 /script/package.sh

bash-32: dirs
	docker run -v $(PWD)/pkg-out:/pkg -v $(PWD)/build32-out:/build -v $(PWD)/script:/script -i -t trenchbroom/build32 -i /bin/bash


clean:
	rm -f pkg-out/*
