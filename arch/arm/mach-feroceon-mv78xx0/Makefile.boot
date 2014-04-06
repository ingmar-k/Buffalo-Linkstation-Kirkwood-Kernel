	zreladdr-y	:= $(shell printf "0x%X" $$(( $(CONFIG_PHYS_OFFSET)+0x00008000)))
params_phys-y	:= $(shell printf "0x%X" $$(( $(CONFIG_PHYS_OFFSET)+0x00000100)))
initrd_phys-y	:= $(shell printf "0x%X" $$(( $(CONFIG_PHYS_OFFSET)+0x00800000)))

