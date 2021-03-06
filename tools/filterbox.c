#include "../src/turH.h"
#include <string.h>


int main(int argc, char* argv[]){

  int NN = atoi(argv[3]);
  
  if (strcmp(argv[1],"--help") == 0){
    printf("Usage: filterbox input output size\n");
    exit(0);
  }
  else{
    printf("%s to %s, and size %d output\n",argv[1],argv[2],NN);
  }

  int Nxnew = NN;
  int Nynew = NN;
  int Nznew = NN+2;
  
  int i,j,k;
  int Nx,Ny,Nz;
  herr_t H5Err;
  hid_t ifileid, idsetid, idspaceid, imspaceid;
  hid_t ofileid, odsetid, odspaceid, omspaceid;
  hsize_t isizem[3], isized[3], imaxsized[3], istart[3], istride[3], icount[3];
  hsize_t osizem[3], osized[3], ostart[3], ostride[3], ocount[3];
  int idx1, idx2;
  float *aux, *aux1;
  int counti, countj, countk;
  float facx, facy, facz;
  
  ifileid = H5Fopen(argv[1], H5F_ACC_RDONLY, H5P_DEFAULT);
  idsetid = H5Dopen(ifileid, "u", H5P_DEFAULT);
  idspaceid = H5Dget_space(idsetid);
  H5Err = H5Sget_simple_extent_dims(idspaceid, isized, imaxsized);
  istart[0] = 0; istart[1] = 0; istart[2] = 0;
  Nx = isized[0]; Ny = isized[1]; Nz = isized[2];

  printf("Size of original: %d x %d x %d\n",Nx, Ny, Nz);
  printf("Size of target: %d x %d x %d\n",Nxnew, Nynew, Nznew);

  if (Nxnew > Nx){
    printf("Wrong sizes. The target field must be smaller\n");
    return -1;
  }
  
  isizem[0] = 1; isizem[1] = Ny; isizem[2] = Nz;
  istride[0] = 1; istride[1] = Ny; istride[2] = Nz;
  icount[0] = 1; icount[1] = 1; icount[2] = 1;

  aux = (float *) malloc(Ny*Nz*sizeof(float));
  aux1 = (float *) malloc(Nynew*Nznew*sizeof(float));


  ofileid = H5Fcreate(argv[2], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  ostart[0] = 0; ostart[1] = 0; ostart[2] = 0;
  osized[0] = Nxnew; osized[1] = Nynew; osized[2] = Nznew;
  osizem[0] = 1; osizem[1] = Nynew; osizem[2] = Nznew;
  ostride[0] = 1; ostride[1] = Nynew; ostride[2] = Nznew;
  ocount[0] = 1; ocount[1] = 1; ocount[2] = 1;
  odspaceid = H5Screate_simple(3,osized,osized);
  odsetid = H5Dcreate(ofileid, "u", H5T_NATIVE_FLOAT, odspaceid,
		      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);


  counti = 0;
  facx = float(Nxnew) / float(Nx);
  facy = float(Nynew) / float(Ny);
  facz = float(Nznew-2) / float(Nz-2);
  
  for (i=0; i<Nx; i++){
    if (i < (Nxnew/2+1) || i > (Nx - (Nxnew/2)) ){
      imspaceid = H5Screate_simple(3,isizem,isizem);
      istart[0] = 0;
      H5Err = H5Sselect_hyperslab(imspaceid,H5S_SELECT_SET,
				  istart,istride,icount,isizem);
      istart[0] = i;
      H5Err = H5Sselect_hyperslab(idspaceid,H5S_SELECT_SET,
				  istart,istride,icount,isizem);
      H5Err = H5Dread(idsetid,H5T_NATIVE_FLOAT,imspaceid,idspaceid,
		      H5P_DEFAULT,aux);
      H5Err = H5Sclose(imspaceid);
      
      countj = 0;
      for (j=0; j<Ny; j++){
	if (j < (Nynew/2+1) || j > (Ny - (Nynew/2)) ){
	  countk = 0;
	  for (k=0; k<Nz; k++){
	    if (k < Nznew){
	      aux1[countj*Nznew + countk] = facx*facy*facz*aux[j*Nz + k];
	      countk += 1;
	    }
	  }
	  countj += 1;
	}
      }
      printf("Plane %d to %d\n",i, counti);
      counti += 1;
    }
    
    omspaceid = H5Screate_simple(3,osizem,osizem);
    ostart[0] = 0;
    H5Err = H5Sselect_hyperslab(omspaceid,H5S_SELECT_SET,
				ostart,ostride,ocount,osizem);
    ostart[0] = counti-1;
    H5Err = H5Sselect_hyperslab(odspaceid,H5S_SELECT_SET,
				ostart,ostride,ocount,osizem);
    H5Err = H5Dwrite(odsetid,H5T_NATIVE_FLOAT,omspaceid,odspaceid,
		     H5P_DEFAULT,aux1);
    H5Err = H5Sclose(omspaceid);    
  }
  
  printf("Counters i: %i, j:%i, k:%i\n",counti, countj, countk);
  
  H5Err = H5Dclose(idsetid);
  H5Err = H5Sclose(idspaceid);
  H5Err = H5Fclose(ifileid);
  H5Err = H5Dclose(odsetid);
  H5Err = H5Sclose(odspaceid);
  H5Err = H5Fclose(ofileid);

  free(aux);
  free(aux1);

}

