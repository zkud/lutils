#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_PATH 256
#define MAX_BUFFER_SIZE 255


void print_header() {
    puts(
        "model | serial number | firmware | "
        "memory occupied | memory all | "
        "interface type | modes" 
    );
}


void clear_buffer(char* pointer) {
    int counter = MAX_BUFFER_SIZE;

    while(counter --> 0) {
        *pointer = 0;
        pointer++;
    }
}


// returns drive total size in bytes
unsigned long long get_sd_size(const char* drive_name) {
    char buffer[MAX_BUFFER_SIZE] = {};
    FILE* sectors_count_file, *sector_size_file;
    unsigned long long size = 0, sector_size = 0;

    strncpy(buffer, "sys/block/", MAX_PATH);
    strncat(buffer, drive_name, MAX_PATH);
    strncat(buffer, "/size", MAX_PATH);
    if(sectors_count_file = fopen(buffer, "r")) {
        fscanf(sectors_count_file, "%llu", &size);
        fclose(sectors_count_file);
    }
    else {
        perror("Error opening sda file: ");
    }

    strncpy(buffer, "sys/block/", MAX_PATH);
    strncat(buffer, drive_name, MAX_PATH);
    strncat(buffer, "/queue/hw_sector_size", MAX_PATH);
    if(sector_size_file = fopen(buffer, "r")) {
        fscanf(sector_size_file, "%llu", &sector_size);
        fclose(sector_size_file);
    }
    else {
        perror("Error opening sda sector size file: ");
    }

    return size * sector_size;
}


unsigned long long get_allocated_size(const char* drive_name) {
    FILE* size_file, *sector_size_file;
    char path[MAX_PATH];
    DIR* sd_directory;
    struct dirent* current_file;
    unsigned long long size = 0, buffer = 0, sector_size = 0;

    strncpy(path, "sys/block/", MAX_PATH);
    strncat(path, drive_name, MAX_PATH);

    // find the all dirs sd* in sd* and sum their size
    if(sd_directory = opendir(path)) {
        while(current_file = readdir(sd_directory)) {
            if(current_file->d_name[0] == 's' && current_file->d_name[1] == 'd') {
                strncpy(path, "sys/block/", MAX_PATH);
                strncat(path, drive_name, MAX_PATH);
                strncat(path, "/", MAX_PATH);
                strncat(path, current_file->d_name, MAX_PATH);
                strncat(path, "/size", MAX_PATH);

                if(size_file = fopen(path, "r")) {
                    fscanf(size_file, "%llu", &buffer);
                    fclose(size_file);

                    size += buffer;
                }
                else {
                    perror("Error opening sd*/sd*/size file: ");
                }
            }
        }

        closedir(sd_directory);
    }
    else {
        perror("Error opening sys/block/sd* directory: ");
    }

    // get sector size
    strncpy(path, "sys/block/", MAX_PATH);
    strncat(path, drive_name, MAX_PATH);
    strncat(path, "/queue/hw_sector_size", MAX_PATH);
    if(sector_size_file = fopen(path, "r")) {
        fscanf(sector_size_file, "%llu", &sector_size);
        fclose(sector_size_file);
    }
    else {
        perror("Error opening sda sector size file: ");
    }

    return size * sector_size;
}


void print_modes(__u16* idw) {
	__u8 pio;
    char pmodes[64] = {0,}, dmodes[128]={0,}, umodes[128]={0,};

	pio = idw[51] >> 8;
	if (pio <= 5) {
		strcat(pmodes, "pio0 ");
		if (pio >= 1) strcat(pmodes, "pio1 ");
		if (pio >= 2) strcat(pmodes, "pio2 ");
	}

	if (idw[49] & 0x100) {
		if (idw[62] | idw[63]) {
			if (idw[62] & 1)	strcat(dmodes,"sdma0 ");
			if (idw[62] & 2)	strcat(dmodes,"sdma1 ");
			if (idw[62] & 4)	strcat(dmodes,"sdma2 ");
			if (idw[62] & 0xf8)	strcat(dmodes,"sdma? ");
			if (idw[63] & 1)	strcat(dmodes,"dma0 ");
			if (idw[63] & 2)	strcat(dmodes,"dma1 ");
			if (idw[63] & 4)	strcat(dmodes,"dma2 ");
			if (idw[63] & 0xf8)	strcat(dmodes,"dma? ");
		}
	}

	if ((idw[49] & 0x800) || (idw[53] & 2)) {
		if ((idw[53] & 2)) {
			if (idw[64] & 1)	strcat(pmodes, "pio3 ");
			if (idw[64] & 2)	strcat(pmodes, "pio4 ");
			if (idw[64] &~3)	strcat(pmodes, "pio? ");
		}
		if (idw[53] & 4) {
			if (idw[88] & 0x001)	strcat(umodes,"udma0 ");
			if (idw[88] & 0x002)	strcat(umodes,"udma1 ");
			if (idw[88] & 0x004)	strcat(umodes,"udma2 ");
			if (idw[88] & 0x008)	strcat(umodes,"udma3 ");
			if (idw[88] & 0x010)	strcat(umodes,"udma4 ");
			if (idw[88] & 0x020)	strcat(umodes,"udma5 ");
			if (idw[88] & 0x040)	strcat(umodes,"udma6 ");
		}
	}

	printf("\tPIO:  %s", pmodes);
	if (*dmodes)
		printf("\n\tDMA:  %s", dmodes);
	if (*umodes)
		printf("\n\tUDMA: %s", umodes);
}


#define TRANSPORT_MAJOR		222 // PATA vs. SATA etc.
#define TRANSPORT_MINOR		223 // minor revision number


void print_interface(__u16 val[])
{
	__u16 major = val[TRANSPORT_MAJOR], minor = val[TRANSPORT_MINOR];
	unsigned int ttype, subtype, transport = 0;

	printf("\tinterface type: ");
	ttype = major >> 12;
	subtype = major & 0xfff;
	transport = ttype;
	switch (ttype) {
		case 0:
			printf("Parallel");
			if (subtype & 1)
				printf(", ATA8-APT");
			break;
		case 1:
			printf("Serial");
			if (subtype & 0x2f) {
				if (subtype & (1<<0))
					printf(", ATA8-AST");
				if (subtype & (1<<1))
					printf(", SATA 1.0a");
				if (subtype & (1<<2))
					printf(", SATA II Extensions");
				if (subtype & (1<<3))
					printf(", SATA Rev 2.5");
				if (subtype & (1<<4))
					printf(", SATA Rev 2.6");
				if (subtype & (1<<5))
					printf(", SATA Rev 3.0");
			}
			break;
		default:
			printf("0x%04x", major);
			break;
	}

    puts("");
}


void print_sd_info(const char* drive_name) {
    char drive_path[MAX_PATH];
    int drive;
    struct hd_driveid hd;
    unsigned long long all_size, allocated_size;

    strncpy(drive_path, "dev/", MAX_PATH);
    strncat(drive_path, drive_name, MAX_PATH);

    drive = open(drive_path, O_RDONLY);
    if(drive <= 0) {
        perror("Error in open sd* :");
        return;
    }

    if (ioctl(drive, HDIO_GET_IDENTITY, &hd) == 0) { 
        puts(drive_name);
        printf("\tmodel: %.*s \n", 40, hd.model);
        printf("\tserial number: %.*s \n", 20, hd.serial_no);
        printf("\tfw revision: %.*s \n", 8, hd.fw_rev);

        all_size = get_sd_size(drive_name);
        printf("\tmemory all (bytes): %llu \n", all_size);

        allocated_size = get_allocated_size(drive_name);
        printf("\tmemory allocated (bytes): %llu \n", allocated_size);

        printf("\tmemory free (bytes): %llu \n", all_size - allocated_size);

        print_interface((__u16*)&hd);

        print_modes((__u16*)&hd);
        puts("");
    } else { 
        perror("Error fetching sd* data: ");
    }

    close(drive);
}


int main(void) {
    char drive_path[MAX_PATH];
    DIR* sys_directory;
    struct dirent* current_file;

    if (chdir("/")) {
        perror("Error changing directory: ");
        return 1;
    }

    // searn in sysfs for sd* devices and print info about them
    if(sys_directory = opendir("sys/block")) {
        while(current_file = readdir(sys_directory)) {
            if(current_file->d_name[0] == 's' && current_file->d_name[1] == 'd') {
                print_sd_info(current_file->d_name);
            }
        }

        closedir(sys_directory);
    }
    else {
        perror("Error opening sys/block/ directory: ");
    }

    return 0;
}

