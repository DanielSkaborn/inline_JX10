#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	int wd;

	unsigned char mapped[128][8];
	unsigned char page;
	unsigned char noofpages=4;
	unsigned char parametervalue[128];
	unsigned char parameterstatus[128];
	unsigned char syxoutblock[11];


	int i, ii, mps, notallset;
	unsigned char channel = 0;

	unsigned char inbuf;
	unsigned char utbuf;
	unsigned char controller;
	size_t nbytes;
	ssize_t x;
//	int x;

	fd = open("/dev/midi1",O_RDONLY, 0);
	wd = open("/dev/midi1",O_WRONLY,0);
//	printf("%x\n",fd);

	if (fd < 0) {
		printf("could not open device\n");
		return 0;
	}

	for (ii=0;ii<8;ii++) {
		for (i=0;i<128;i++) {
			mapped[i][ii]=0;
			parameterstatus[i]=0;
		}
		mapped[0x18][ii]=0xFF; // page select!
		mapped[0x73][ii]=0xFE;
	}

	// list of non used parameters
	parameterstatus[0]=2;
	parameterstatus[1]=2;
	parameterstatus[2]=2;
	parameterstatus[3]=2;
	parameterstatus[4]=2;
	parameterstatus[5]=2;
	parameterstatus[6]=2;
	parameterstatus[7]=2;
	parameterstatus[8]=2;
	parameterstatus[9]=2;
	parameterstatus[10]=2;
	parameterstatus[23]=2;
	parameterstatus[24]=2;
	parameterstatus[25]=2;
	parameterstatus[57]=2;
	for (i=59;i<128;i++)
		parameterstatus[i]=2;


	for (i=0;i<noofpages+1;i++) {
		//sliders
		mapped[0x10][i] = 47; // A ENV1
		mapped[0x11][i] = 48; // D
		mapped[0x12][i] = 49; // S
		mapped[0x13][i] = 50; // R
		mapped[0x14][i] = 52; // A ENV2
		mapped[0x15][i] = 53; // D
		mapped[0x16][i] = 54; // S
		mapped[0x17][i] = 55; // R
		//joystick
        	mapped[0x31][i] = 34; // VCF
        	mapped[0x32][i] = 35; // RES
	}

	//rattar
	mapped[0x19][0] = 51; // ENV1 keyfollow
	mapped[0x1a][0] = 56; // ENV2 keyfollow 
	mapped[0x1b][0] = 30; // mixer ENV mod depth
	mapped[0x1c][0] = 36; // VCF mod depth 

	mapped[0x1d][0] = 44; // LFO WAVE
	mapped[0x1e][0] = 45; // LFO DELAY
	mapped[0x1f][0] = 46; // LFO SPEED
	mapped[0x30][0] = 43; // CHORUS

//----------------------------

        mapped[0x19][1] = 11; // DCO1 range
        mapped[0x1a][1] = 12; // DCO1 wave
        mapped[0x1b][1] = 13; // DCO1 tune
        mapped[0x1c][1] = 26; // DCO dynamics

        mapped[0x1d][1] = 14; // DCO1 env depth
        mapped[0x1e][1] = 15; // DCO1 lfo depth
        mapped[0x1f][1] = 28; // DCO1 mix
        mapped[0x30][1] = 32; // mixer env mode

//---------------------------

	mapped[0x19][2] = 16; // DCO2 range
        mapped[0x1a][2] = 17; // DCO2 wave
        mapped[0x1b][2] = 19; // DCO2 tune
        mapped[0x1c][2] = 20; // DCO2 finetune

        mapped[0x1d][2] = 21; // DCO2 env depth
        mapped[0x1e][2] = 22; // DCO2 lfo depth
        mapped[0x1f][2] = 29; // DCO2 mix
	mapped[0x30][2] = 18; // DCO2 crossmod

//----------------------------

	mapped[0x19][3] = 33; // HPF
        mapped[0x1a][3] = 39; // VCF dynamics
        mapped[0x1b][3] = 38; // VCF keyfollow
        mapped[0x1c][3] = 37; // VCF env depth

        mapped[0x1d][3] = 41; // VCA level
        mapped[0x1e][3] = 42; // VCA dynamics
        mapped[0x1f][3] = 58; // VCA env mode
        mapped[0x30][3] = 40; // VCF env mode

//----------------------------

        mapped[0x19][4] = 31; // mixer dynamics
        mapped[0x1a][4] = 32; // mixer env mode
        mapped[0x1b][4] = 27; // DCO env mode
//        mapped[0x1c][4] = ; // 

        mapped[0x1d][4] = 34; // VCF 
        mapped[0x1e][4] = 35; // RES
//        mapped[0x1f][4] = ; // 
//        mapped[0x30][4] = ; // 



	i=0; mps=0;
	channel=0;

	printf("\n\nInline MIDI ControlChange to JX10 syx parameter set\nWritten by Daniel Skaborn 2011\n\n");

	printf("Mapping setup:\n\n         0    1    2    3    4    5    6    7");
	for (i=0;i<128;i++) {
		if (mapped[i][0]) {
			printf("\nC %02X >  ",i);
			for (ii=0;ii<8;ii++)
				if (mapped[i][ii]==0) {
					printf("     ");
				} else {
					printf("P%02X  ", mapped[i][ii]);
				}
		}
	}
	printf("\n\n[0xFF = Page, 0xFE = save syx]\n\n");

	page=0;

	while (1) {
		i++;
		x = read(fd, &inbuf, sizeof(inbuf) );

		switch(mps) {
			case 0: // normal
				if (inbuf == (0xB0+channel)) {
					mps=1;
				} else {
					write(wd, &inbuf, sizeof(inbuf));
				}
				break;
			case 1: // got ctrl
//				if(inbuf && 0x80) { controller = 0x80; mps=0; break; } // invalid
				controller = inbuf;
				mps=2;
//				printf("%X\n",controller);
				break;
			case 2: 
//				if (inbuf && 0x80) { controller = 0x80; mps=0; break; } // invalid
				if (mapped[controller][page]==0xFF) { // Change Page!
					if (page!=inbuf/(0x7F/noofpages)) {
						page=inbuf/(0x7F/noofpages);
						printf("Page = %d\n",page);
					}
					mps=0;
					break;
				}
				if (mapped[controller][page]==0xFE) { // Save SYX file
					notallset=0;
					for (i=0;i<128;i++) 
						if (parameterstatus[i]==0) { notallset=1; printf("%d ",i); }
					if (notallset==1) {
						printf("Warning not all parameters has been set!\nNo file saved!\n");
						mps=0;
						break;
					}
					for (i=0;i<128;i++) {
						// write syx file!
						syxoutblock[0]=0xF0;
						syxoutblock[1]=0x41;
						syxoutblock[2]=0x36;
						syxoutblock[3]=channel;
						syxoutblock[4]=0x24;
						syxoutblock[5]=0x20;
						syxoutblock[6]=0x01;
						syxoutblock[9]=0xF7;
						syxoutblock[10]='\0';
						for(i=0;i<128;i++) {
							if (parameterstatus!=2) {
								syxoutblock[7]=i;
								syxoutblock[8]=parametervalue[i];
								// TODO: write out syxoutblock here
							}
						}
						printf("SYX write not implemented yet!\n");
					}
					mps=0;
					break;
				}
				if (mapped[controller]!=0) {
					utbuf=0xF0; // sysex
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0x41; // roland
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0x36; // IPR
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=channel;
 					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0x24; // JX10 / MKS70
 					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0x20; // lecel
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0x01; // group
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=mapped[controller][page];
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=inbuf;
					write(wd, &utbuf, sizeof(utbuf));
					utbuf=0xF7; // end of sysex
					write(wd, &utbuf, sizeof(utbuf));
					printf("[C%02X > P%02X V%02X]\n", controller, mapped[controller][page], inbuf);
					parametervalue[mapped[controller][page]]=inbuf;
					parameterstatus[mapped[controller][page]]=1;
				} else {
					utbuf=0xB0+channel;
					write(wd, &utbuf, sizeof(utbuf));
					write(wd, &controller, sizeof(controller));
					write(wd, &inbuf, sizeof(inbuf));
					printf("[C%02X]\n",controller);
				}
				mps=0;
				break;
			default:
				mps=0;
		}
	}
	return 0;
}
