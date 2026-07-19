obj-m += BeamScan_loader.o
BeamScan_loader-objs := main.o ReadFile.o

PWD := $(CURDIR)

all: clean module user user_model copy-config remove install

USER_APP := beamscan
USER_SRCS := beamscan.c


$(USER_APP): $(USER_SRCS)
	gcc -Wall -Wextra -O2 $(DEFS) -o $@ $^ -lm

USER_MODEL_APP := beamscan_model
USER_MODEL_SRCS := beamscan_model.c


$(USER_MODEL_APP): $(USER_MODEL_SRCS)
	gcc -Wall -Wextra -O2 $(DEFS) -o $@ $^ -lm

# ---------------- Kernel ----------------

DEBUG_OUT_MATRIX_CSV ?= y
GRAFICS ?= y

DEFS := \
	$(if $(filter y,$(DEBUG_OUT_MESSAGE)),-DDEBUG_OUT_MESSAGE,) \
	$(if $(filter y,$(DEBUG_OUT_MATRIX_CSV)),-DDEBUG_OUT_MATRIX_CSV,) \
	$(if $(filter y,$(MULTI_THREADING)),-DMULTI_THREADING,) \
	$(if $(filter y,$(BEAM_2D_ON)),-DBEAM_2D_ON,) \
	$(if $(filter y,$(BEAM_1D_ON)),-DBEAM_1D_ON,) \
	$(if $(filter y,$(GRAFICS)),-DGRAFICS,) \
	-DTHETTA_NUMBER_POINT=241 \
	-DPHI_NUMBER_POINT=241 \
	-DNUMBER_X=5 \
	-DNUMBER_Y=6 \
	-DNUMBER_SIGNALS=2 \
	-DSNAPSHOTS=6 \
	-DNUMB_CONTER_FOR=1 \
	-DMAX_LINE=256

ccflags-y := -Wall -Wextra $(DEFS)

# ---------------- Targets ----------------

module:
	sudo apt install libopenblas-dev
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user: $(USER_APP)
user_model: $(USER_MODEL_APP)

copy-config:
	@echo "Создание /etc/configs..."
	sudo mkdir -p /etc/configs
	sudo mkdir -p /usr/local/bin

	@if [ -f Beam.params ]; then \
		sudo cp Beam.params /etc/configs/; \
		sudo cp ArrayStruct.params /etc/configs/; \
		echo "Файлы конфигурации скопированы"; \
	fi

	@if [ -f $(USER_APP) ]; then \
		sudo cp $(USER_APP) /usr/local/bin/; \
		sudo chmod +x /usr/local/bin/$(USER_APP); \
		echo "beamscan установлен"; \
	fi

	@if [ -f $(USER_MODEL_APP) ]; then \
		sudo cp $(USER_MODEL_APP) /usr/local/bin/; \
		sudo chmod +x /usr/local/bin/$(USER_MODEL_APP); \
		echo "beamscan_model установлен"; \
	fi

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(USER_APP) $(USER_MODEL_APP)

install:
	sudo insmod BeamScan_loader.ko D1=1 D2=0 Model=1

remove:
	sudo rmmod BeamScan_loader
#	sudo killall beamscan

status:
	lsmod | grep BeamScan_loader

log:
	sudo dmesg | tail -60

.PHONY: all module user clean install remove status log copy-config