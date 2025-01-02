image_base=registry.infra.smartparking.kz/parqour_anpr_base:3.0
image=registry.infra.smartparking.kz/parqour_anpr_app
tag=v2.0.0
dev_tag=dev

build_base:
	docker build -t ${image_base} -f dockerfiles/DockerfileBase .

build_dev:
	docker build -t ${image}:${dev_tag} -f dockerfiles/DockerfileDev .

run_dev:
	xhost local:root && docker run -d -it --rm --cap-add sys_ptrace -p127.0.0.1:2222:22 \
            --gpus all -e DISPLAY="$DISPLAY" \
            -v /tmp/.X11-unix:/tmp/.X11-unix \
            --name parking_cpp_local \
            ${image}:${dev_tag}

run_dev_2:
	docker run -d -it --rm --cap-add sys_ptrace -p0.0.0.0:2222:22 \
		--gpus all --name parking_cpp_remote ${image}:${dev_tag}

build_app:
	docker build -t ${image}:${tag} -f dockerfiles/DockerfileApp .

stop_dev:
	docker stop parking_cpp_local