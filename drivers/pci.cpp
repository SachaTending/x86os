#include <pci.hpp>
#include <io.h>
#include <logging.hpp>
#include <libc.hpp>
#include <pci_ven_db.h>

static Logging log("PCI");

uint32_t pci_size_map[100];

uint32_t PCI::Read(pci_dev_t dev, uint32_t field) {
	// Only most significant 6 bits of the field
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outl(PCI_CONFIG_ADDRESS, dev.bits);

	// What size is this field supposed to be ?
	uint32_t size = pci_size_map[field];
	if(size == 1) {
		// Get the first byte only, since it's in little endian, it's actually the 3rd byte
		uint8_t t =inb(PCI_CONFIG_DATA + (field & 3));
		return t;
	}
	else if(size == 2) {
		uint16_t t = inw(PCI_CONFIG_DATA + (field & 2));
		return t;
	}
	else if(size == 4){
		// Read entire 4 bytes
		uint32_t t = inl(PCI_CONFIG_DATA);
		return t;
	}
	return 0xffff;
}

void PCI::Write(pci_dev_t dev, uint32_t field, uint32_t value) {
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	// Tell where we want to write
	outl(PCI_CONFIG_ADDRESS, dev.bits);
	// Value to write
	outl(PCI_CONFIG_DATA, value);
}

uint32_t PCI::GetType(pci_dev_t dev) {
	uint32_t t = PCI::Read(dev, PCI_CLASS) << 8;
	return t | PCI::Read(dev, PCI_SUBCLASS);
}


uint32_t get_secondary_bus(pci_dev_t dev) {
	return PCI::Read(dev, PCI_SECONDARY_BUS);
}

uint32_t pci_reach_end(pci_dev_t dev) {
	uint32_t t = PCI::Read(dev, PCI_HEADER_TYPE);
	return !t;
}
pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type);
pci_dev_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;

	if(PCI::Read(dev,PCI_VENDOR_ID) == PCI_NONE)
		return dev_zero;

	pci_dev_t t = pci_scan_function(vendor_id, device_id, bus, device, 0, device_type);
	if(t.bits)
		return t;

	if(pci_reach_end(dev))
		return dev_zero;

	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		if(PCI::Read(dev,PCI_VENDOR_ID) != PCI_NONE) {
			t = pci_scan_function(vendor_id, device_id, bus, device, function, device_type);
			if(t.bits)
				return t;
		}
	}
	return dev_zero;
}

pci_dev_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type) {
	for(int device = 0; device < DEVICE_PER_BUS; device++) {
		pci_dev_t t = pci_scan_device(vendor_id, device_id, bus, device, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;
	dev.function_num = function;
	// If it's a PCI Bridge device, get the bus it's connected to and keep searching
	if(PCI::GetType(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(vendor_id, device_id, get_secondary_bus(dev), device_type);
	}
	// If type matches, we've found the device, just return it
	if(device_type == -1 || device_type == PCI::GetType(dev)) {
		uint32_t devid  = PCI::Read(dev, PCI_DEVICE_ID);
		uint32_t vendid = PCI::Read(dev, PCI_VENDOR_ID);
		if(devid == device_id && vendor_id == vendid)
			return dev;
	}
	return dev_zero;
}

pci_dev_t PCI::Get(uint16_t vendor_id, uint16_t device_id, int device_type) {

	pci_dev_t t = pci_scan_bus(vendor_id, device_id, 0, device_type);
	if(t.bits)
		return t;

	// Handle multiple pci host controllers
	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		pci_dev_t dev = {0};
		dev.function_num = function;

		if(PCI::Read(dev, PCI_VENDOR_ID) == PCI_NONE)
			break;
		t = pci_scan_bus(vendor_id, device_id, function, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

void PCI::BusMasterEnable(pci_dev_t dev) {
	uint32_t pci_command_reg = PCI::Read(dev, PCI_COMMAND);
    if(!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        PCI::Write(dev, PCI_COMMAND, pci_command_reg);
    }
}
extern bool g_ACER;
const char *venID_to_string(uint32_t venID) {
	if (venID == 0x1234 or g_ACER) {
		return "idk";
	}
	for (int i=0;i<sizeof(pci_db);i++) {
		//printf("0x%x 0x%x\n", i, venID);
		if (pci_db[i].vid == venID) {
			if (strlen(pci_db[i].name) > 64) return "Too long company name.";
			return pci_db[i].name;
		}
	}
}

const char *pci_class_to_str(uint32_t class2, uint32_t subclass) {
	switch (class2)
	{
		case 0x01:
			switch (subclass)
			{
				case 0x06:
					return "AHCI Controller";
				
				default:
					return "Mass storage - other";
			}
		
		default:
			return "Unknown or not defined.";
	}
}

void PCI::Init() {
    // Init size map
	pci_size_map[PCI_VENDOR_ID] =	2;
	pci_size_map[PCI_DEVICE_ID] =	2;
	pci_size_map[PCI_COMMAND]	=	2;
	pci_size_map[PCI_STATUS]	=	2;
	pci_size_map[PCI_SUBCLASS]	=	1;
	pci_size_map[PCI_CLASS]		=	1;
	pci_size_map[PCI_CACHE_LINE_SIZE]	= 1;
	pci_size_map[PCI_LATENCY_TIMER]		= 1;
	pci_size_map[PCI_HEADER_TYPE] = 1;
	pci_size_map[PCI_BIST] = 1;
	pci_size_map[PCI_BAR0] = 4;
	pci_size_map[PCI_BAR1] = 4;
	pci_size_map[PCI_BAR2] = 4;
	pci_size_map[PCI_BAR3] = 4;
	pci_size_map[PCI_BAR4] = 4;
	pci_size_map[PCI_BAR5] = 4;
	pci_size_map[PCI_INTERRUPT_LINE]	= 1;
	pci_size_map[PCI_SECONDARY_BUS]		= 1;
	pci_dev_t dev = {0};
	dev.bus_num = 0;
	dev.device_num = 0;
	dev.function_num = 0;
	for (dev.bus_num = 0; dev.bus_num < 31; dev.bus_num++) {
		for (dev.device_num = 0; dev.device_num < 31; dev.device_num++) {
			for (dev.function_num = 0; dev.function_num < 7; dev.function_num++) {
				if (PCI::Read(dev, PCI_VENDOR_ID) != 0xffff) {
					log.info("bus=%u dev=%u func=%u venID=0x%04x(%s) devID=0x%04x class=0x%01x", dev.bus_num, dev.device_num, dev.function_num, PCI::Read(dev, PCI_VENDOR_ID), venID_to_string(PCI::Read(dev, PCI_VENDOR_ID)), PCI::Read(dev, PCI_DEVICE_ID), PCI::Read(dev, PCI_CLASS));
					if (PCI::Read(dev, PCI_SUBCLASS) != 128) {
						printf(" subclass=0x%x", PCI::Read(dev, PCI_SUBCLASS));
						if (!(strlen(pci_class_to_str(PCI::Read(dev, PCI_CLASS), PCI::Read(dev, PCI_SUBCLASS))) > 30)) printf(" type=%s", pci_class_to_str(PCI::Read(dev, PCI_CLASS), PCI::Read(dev, PCI_SUBCLASS)));
					}
					printf("\n");
				}
			}
		}
	}
}