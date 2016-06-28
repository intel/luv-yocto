all:
	$(MAKE) -C $$KERNEL/tools/testing/nvdimm M=$$KERNEL/tools/testing/nvdimm/

modules_install:
	$(MAKE) -C $$KERNEL M=$$KERNEL/tools/testing/nvdimm/ modules_install

.PHONY: clean
clean:
	rm -f $$KERNEL/tools/testing/nvdimm/*.o $$KERNEL/tools/testing/nvdimm/*.ko $$KERNEL/tools/testing/nvdimm/*.mod.c $$KERNEL/tools/testing/Module.symvers $$KERNEL/tools/testing/Module.markers $$KERNEL/tools/testing/modules.order
