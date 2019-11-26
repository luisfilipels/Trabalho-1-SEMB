#include<stdio.h>
#include<stdlib.h>

unsigned char getBit(unsigned char *v, unsigned char i, unsigned char j){
  unsigned short m=0,n=0;
  unsigned char mask = 1;
  m = (i*160+j)/8;
  n = (i*160+j)%8;
  mask = mask << n;
  return ((v[m] & mask) >> n);
}

int main(){
  printf("iniciando");
  unsigned char img[2400];
  FILE *frd;
  unsigned char c;
  frd = fopen("arquivo.txt","r"); //arquivo contendo a saida

  for(int i=0;i<2400;++i){
    fscanf(frd, "%d ", &img[i]);
  }

  FILE *fp;

  fp = fopen("img.pgm", "w+");
  fprintf(fp, "P2\n160 120\n255\n");
  printf("Check0");
  for(int i=0;i<120;++i){
    for(int j=0;j<160;++j){
      if(getBit(img,i,j) == 1){
        fprintf(fp,"%d ", 255);
      }else{
	    fprintf(fp,"%d ", 0);
      }
    }
  }
  fclose(fp);
  fclose(frd);
  return 0;
}
