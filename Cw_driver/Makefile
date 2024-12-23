# Указываем директорию с исходниками ядра
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd) //определение переменной PWD

     .PHONY: all clean //объявление об отсутствии связи с файлами

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
//указывает, что модуль должен быть собран из текущей директории
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions //удаления всех временных и сгенерированных файлов

obj-m := mouse_driver.o
