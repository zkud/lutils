import os
import re
import typing


# configs
PCI_PATH = 'sys/bus/pci/devices/'
PCI_CODES = 'usr/share/misc/pci.ids'


def fill_dicts() -> typing.Tuple[dict, dict]:
    """
    creates dicts like code : name for PCI devices / vendors
    """
    vendors: dict = {}
    devices: dict = {}
    vendor_root = ''

    # parse pci.ids file using its style
    os.chdir('/')
    with open(PCI_CODES, "r") as codes:
        for line in codes.readlines():
            # if vendor
            if re.match(R"([a-zA-Z0-9]+)", line):
                vendor_id = re.match(r"([a-zA-Z0-9]+)", line).group(0).strip()
                vendor_root = '0x' + vendor_id
                vendors[vendor_root] = line[len(vendor_id) + 1:].strip()
            # if device
            elif re.match(r"[\t]([a-zA-Z0-9]+)", line):
                device_id = re.match(r"[\t]([a-zA-Z0-9]+)", line).group(0).strip()
                devices[vendor_root + '0x' + device_id] = line[len(device_id) + 2:].strip()

    return vendors, devices
                

def main():
    output_list = [
        [
            '    path    ',
            'vendor',
            'device',
            '  vendor name  ',
            '  device name  '
        ]
    ]

    vendors, devices = fill_dicts()

    # visit all devices in in devices list
    os.chdir('/')
    for device_path in os.listdir(PCI_PATH):
        # add info about device
        with open(PCI_PATH + device_path + '/device', "r") as device_file:
            with open(PCI_PATH + device_path + '/vendor', "r") as vendor_file:
                device_id = device_file.read().strip()
                vendor_id = vendor_file.read().strip()
                
                device_name = ''
                vendor_name = ''
                try:
                    device_name = devices[vendor_id + device_id]
                except KeyError:
                    device_name = 'not found'
                try:
                    vendor_name = vendors[vendor_id]
                except KeyError:
                    vendor_name = 'not found'

                output_list.append(
                    [device_path, device_id, vendor_id, vendor_name, device_name]
                )

    # print devices list
    for device in output_list:
        print(' | '.join(device))


if __name__ == '__main__':
    main()
