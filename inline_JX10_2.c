#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	FILE *infil;
	FILE *utfil;

	int fd;
	int wd;

	unsigned char mapped[128][8];
	unsigned char page, rd;
	unsigned char noofpages=4;
	unsigned char parametervalue[128];
	unsigned char parameterstatus[128];
	unsigned char syxoutblock[11];

	int a,b,c,d,e,f,g,h, cntrl;
	unsigned char device[255];
	unsigned char tmp[255];
	int syxfilecounter = 0;

	int i, ii, mps, notallset;
	unsigned char channel = 0;
	int infilstate = 0;

	unsigned char inbuf;
	unsigned char utbuf;
	unsigned char controller;
	size_t nbytes;
	ssize_t x;

       printf("\n\nInline MIDI ControlChange to JX10 syx parameter set\nWritten by Daniel Skaborn 2011\n\n");


	infil=fopen("mapping.jxmp","r");

        for (ii=0;ii<8;ii++) {
                for (i=0;i<128;i++) {
                        mapped[i][ii]=0;
                        parameterstatus[i]=0;
                }
        }

	if (infil==NULL) {
		printf("Could not open mapping file: mapping.jxmp\n");
		return -1;
	}

	infilstate = 0;
	i=0;
	while(!feof(infil)) {
		rd=fgetc(infil);
		switch(infilstate) {
			case 0: // read device
				if ((rd=='\n') | (rd=='\r')) {
					device[i] = '\0';
					infilstate=1; i=0;
					break;
				}
				device[i]=rd; i++;
				break;
			 case 1: // read midi channel
				if ((rd=='\n') | (rd=='\r')) {
					tmp[i]='\0'; i=0;
					sscanf(tmp,"%d",&channel);
					channel--;
					if (channel<0) channel = 0;
					if (channel>15) channel = 15;
					printf("MIDI channel = %d\n", channel+1);
					infilstate=2;
					break;
				}
				tmp[i]=rd; i++;
				break;
			case 2: // read number of pages
                                if ((rd=='\n') | (rd=='\r')) {
                                        tmp[i]='\0'; i=0;
                                        sscanf(tmp,"%d",&noofpages);
                                        noofpages--;
                                        if (noofpages<1) noofpages = 1;
                                        if (noofpages>8) noofpages = 8;
                                        printf("pages = %d\n", noofpages+1);
                                        infilstate=3;
                                        break;
                                }
                                tmp[i]=rd; i++;
                                break;
			case 3: // read mapping
				if ((rd=='\n') | (rd=='\r')) {
                                        tmp[i]='\0'; i=0;
					a=0; b=0; c=0;
                                        sscanf(tmp,"0x%x %d %d %d %d %d %d %d %d",&cntrl, &a,&b,&c,&d,&e,&f,&g,&h);
					mapped[(unsigned char)(cntrl)][0]=(unsigned char)(a);
 					mapped[(unsigned char)(cntrl)][1]=(unsigned char)(b);
					mapped[(unsigned char)(cntrl)][2]=(unsigned char)(c);
					mapped[(unsigned char)(cntrl)][3]=(unsigned char)(d);
 					mapped[(unsigned char)(cntrl)][4]=(unsigned char)(e);
 					mapped[(unsigned char)(cntrl)][5]=(unsigned char)(f);
					mapped[(unsigned char)(cntrl)][6]=(unsigned char)(g);
					mapped[(unsigned char)(cntrl)][7]=(unsigned char)(h);
                       			if (a==0xFF) { // no more to read.
						infilstate=4;
					}
                                        break;
                                }
                                tmp[i]=rd; i++;
                                break;
			case 4:
				break;
		}
	}
	fclose(infil);

	infil=fopen("counter.cnt","r");
	if (infil==NULL) {
		printf("No counterfile, will number .syx files from zero.\n");
		syxfilecounter=0;
	} else {
		fscanf(infil, "%d", &syxfilecounter);
		fclose(infil);
	}


        fd = open(device,O_RDONLY, 0);
        wd = open(device,O_WRONLY, 0);


	if (fd < 0) {
		printf("could not open device: "); printf("%s", device); printf("\n");
		return -1;
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

/*
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
*/


	i=0; mps=0;

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

						sprintf(tmp, "JX10_%05d.syx", syxfilecounter);
						utfil = fopen(tmp,"w");
						printf("Writing SYX file: %s\n",tmp);
						for(i=0;i<128;i++) {
							if (parameterstatus[i]!=2) {
								syxoutblock[7]=i;
								syxoutblock[8]=parametervalue[i];
								for (ii=0;ii<10;ii++) {
									fputc(syxoutblock[ii],utfil);
									//printf("%02X ",syxoutblock[ii]);
								}

							}
						}
						fclose(utfil);
						syxfilecounter++;
						utfil = fopen("counter.cnt","w");
						fprintf(utfil, "%d\n",syxfilecounter);
						fclose(utfil);
						printf("File write completed\n");
//						printf("SYX write not implemented yet!\n");
					}
					mps=0;
					break;
				}
				if (mapped[controller][page]!=0) {
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
