/*  skeletonapp.cc

    Mark Jenkinson, FMRIB Image Analysis Group

    Copyright (C) 2003 University of Oxford  */

/*  CCOPYRIGHT  */

// Skeleton application framework for using newimage

#define _GNU_SOURCE 1
#define POSIX_SOURCE 1

#include "newimage/newimageall.h"
#include "miscmaths/miscmaths.h"
#include "utils/options.h"

using namespace std;
using namespace NEWMAT;
using namespace MISCMATHS;
using namespace NEWIMAGE;
using namespace Utilities;

// The two strings below specify the title and example usage that is
//  printed out as the help or usage message

string title="lesion_filling \nCopyright(c) 2012-2019, University of Oxford (Mark Jenkinson & Taylor Hanayik)";
string examples="lesion_filling [options] -i <intensity image> -l <lesion mask image> -o <output/filled image>";

// Each (global) object below specificies as option and can be accessed
//  anywhere in this file (since they are global).  The order of the
//  arguments needed is: name(s) of option, default value, help message,
//       whether it is compulsory, whether it requires arguments
// Note that they must also be included in the main() function or they
//  will not be active.

Option<bool> verbose(string("-v,--verbose"), false,
                     string("switch on diagnostic messages"),
                     false, no_argument);
Option<bool> writecomps(string("-c,--components"), false,
                     string("save all lesion components as volumes"),
                     false, no_argument);
Option<bool> help(string("-h,--help"), false,
                  string("display this message"),
                  false, no_argument);
Option<string> lesmaskname(string("-l,--lesionmask"), string(""),
                           string("filename of lesion mask image"),
                           true, requires_argument);
Option<string> wmmaskname(string("-w,--wmmask"), string(""),
                          string("filename of white matter mask image"),
                          false, requires_argument);
Option<string> involname(string("-i,--in"), string(""),
                         string("input image filename (e.g. T1w image)"),
                         true, requires_argument);
Option<string> outname(string("-o,--out"), string(""),
                       string("output filename (lesion filled image)"),
                       true, requires_argument);
int nonoptarg;

////////////////////////////////////////////////////////////////////////////

// Local functions

int urand(int min, int max)
{
    int MAXRAND=2147483647;  // = 2^31 - 1
    int mrand=Max(Min(ceil((double(random())/MAXRAND)*(max-min+1)),(max-min+1)),1)+min-1;
    return mrand;
}


// for example ... print difference of COGs between 2 images ...
int do_work(int argc, char* argv[])
{
    // variables from the matlab code...
    // [im,dims,pixdims]=read_avw(structim);
    // compb=read_avw(outerborder_comp);
    // comp=read_avw(lesioncomp);
    // lesinb=read_avw(innerborder);
    // lesinreg=read_avw(innerregion);

    volume<int> tmpcomp;
    volume4D<float> invol;
    volume<float> lesionmask, comp, wmmask;
    read_volume(lesionmask,lesmaskname.value());
    read_volume4D(invol,involname.value());
    if (wmmaskname.set()) {
        read_volume(wmmask,wmmaskname.value());
    } else {
        wmmask=lesionmask*0.0f + 1.0f;
    }
    // check if stuct image and lesion mask image are the same size
    if (!samesize(invol, lesionmask, 3, true)) {
        cerr << "Structural and lesion mask image sizes do not match!" << endl;
        exit(EXIT_FAILURE);
    }
    // check if stuct image and wm mask image are the same size
    if (!samesize(invol, wmmask, 3, true)) {
        cerr << "Structural and white matter mask image sizes do not match!" << endl;
        exit(EXIT_FAILURE);
    }
    // check if lesion mask image and wm mask image are the same size
    if (!samesize(lesionmask, wmmask, 3, true)) {
        cerr << "Lesion mask and white matter mask image sizes do not match!" << endl;
        exit(EXIT_FAILURE);
    }

    if (verbose.value()) { cout << "Read in images" << endl; }
    // connectedcomp ../Lesion_mask_2struct_bin.nii.gz lesion_comp
    tmpcomp = connected_components(lesionmask);
    copyconvert(tmpcomp,comp);
    if (verbose.value()) { cout << "Counted lesions in mask : total = " << comp.max() << endl; }
    // fslmaths ../Lesion_mask_2struct_bin.nii.gz -dilF -sub ../Lesion_mask_2struct_bin.nii.gz ANCO_outerborder.nii.gz
    volume<float> box3kernel, outborder, compb;
    box3kernel=box_kernel(3,3,3);
    outborder = morphfilter(lesionmask,box3kernel,"dilate") - lesionmask;
    outborder.binarise(0.5f);
    if (verbose.value()) { cout << "Calculated outer border mask" << endl; }
    // fslmaths lesion_comp.nii.gz -dilD lesion_comp_dil
    // fslmaths lesion_comp_dil.nii.gz -mul ANCO_outerborder.nii.gz ANCO_outerborder_comp
    compb = morphfilter(comp,box3kernel,"dilateD");
    compb *= outborder;
    compb *= wmmask;
    if (verbose.value()) { cout << "Calculated outer border components" << endl; }
    volume<float> lesinb, lesinreg;
    // fslmaths ../Lesion_mask_2struct_bin.nii.gz -ero -sub ../Lesion_mask_2struct_bin.nii.gz -abs -thr 0.5 -bin ANCO_innerborder
    lesinb = morphfilter(lesionmask,box3kernel,"erodeS");
    lesinb = binarise(lesionmask - lesinb,0.5f);  // this is the inner border mask
    // fslmaths ../Lesion_mask_2struct_bin.nii.gz -sub ANCO_innerborder ANCO_innerregion
    lesinreg = lesionmask - lesinb;
    if (verbose.value()) { cout << "Calculated inner border and region masks" << endl; }

    int maxcomp = comp.max();

    // In the matlab code there was:
    // % check if any points in the inner boundary are more than
    // % one voxel from any other points, and if so, make them part
    // % of the inner region instead
    // but this seems pointless given the definition above

    // for each lesion fill the interior region
    for (int compnum=1; compnum<=maxcomp; compnum++) {
        if (verbose.value()) { cout << "Component " << compnum << " of " << maxcomp << endl; }
        volume<float> mask, maskinb, maskinr;
        mask=binarise(compb, compnum-0.5f, compnum+0.5f);
        maskinb=binarise(comp, compnum-0.5f, compnum+0.5f);
        maskinb*=lesinb;
        maskinr=binarise(comp, compnum-0.5f, compnum+0.5f);
        maskinr*=lesinreg;

        // prep component num for string printing
        std::stringstream les_comp_num;

        // if mask is all zeros throw an error. This is likely due to a lesion being drawn outside of the white matter mask
        // report the COG of the offending lesion so that the user can verify if this lesion is within the WM mask
        if (mask.max() < 1) {
            // prep voxel coordinates for string printing (work-around for std::to_string not working)
            std::stringstream les_vox_coor_x;
            std::stringstream les_vox_coor_y;
            std::stringstream les_vox_coor_z;

            // store voxel coordinates
            les_vox_coor_x << MISCMATHS::round(maskinr.cog()[0]);
            les_vox_coor_y << MISCMATHS::round(maskinr.cog()[1]);
            les_vox_coor_z << MISCMATHS::round(maskinr.cog()[2]);

        	cerr << "ERROR: Lesion at voxel coordinate: "  << les_vox_coor_x.str() << " " << les_vox_coor_y.str() << " " << les_vox_coor_z.str() << " is not within white matter mask" << endl;
        	exit(EXIT_FAILURE);
        }

        if (writecomps.value()){
            // store comp num
            les_comp_num << compnum;
        	// write out each component
        	if (verbose.value()) { cout << "Saving component " << compnum << " as " << appendFSLfilename(outname.value(),"_lesmask_comp_" + les_comp_num.str()) << endl; }
        	save_volume(mask[0],appendFSLfilename(outname.value(),"_lesmask_comp_" + les_comp_num.str()));
        }

        int nin=MISCMATHS::round(maskinr.sum());
        if (nin>0) {
            // extra intensity values from only within the mask
            Matrix vals, sval;
            vals = invol.matrix(mask);
            vals=vals.t();
            if (verbose.value()) { cout << "Cols = " << vals.Ncols() << " and rows = " << vals.Nrows() << endl; }
            int nvox;
            nvox = vals.Nrows();
            if (vals.Ncols()>vals.Nrows()) { cerr << "WRONG ASSUMPTION ABOUT MATRIX METHOD IN NEWIMAGE!!!!!!" << endl << "Cols = " << vals.Ncols() << " and rows = " << vals.Nrows() << endl; }
            sval=vals;
            SortAscending(sval);
            if (verbose.value()) { cout << "Calculating cummulative distribution" << endl; }
            ColumnVector borders(nvox), cumnb(nvox);  // cumnb is cummulative distribution
            float stot=sval.Sum(), cumsum=0.0;
            int cidx=1, prevnb=0;
            for (int nb=1; nb<=nvox; nb+=5) {  // sample cummulative dist at every 5th sample for a bit of smoothing
                for (int midx=prevnb+1; midx<=nb; midx++) { cumsum+=sval(midx,1); }
                prevnb=nb;
                borders(cidx)=sval(nb,1);
                cumnb(cidx++)=cumsum/stot;
            }
            int maxcidx=cidx-1;

            if (verbose.value()) { cout << "Random sampling..." << endl; }
            ColumnVector idx(nin), newvals(nin);
            for (int midx=1; midx<=nin; midx++) { idx(midx)=urand(1,nin)/((float) nin); }  // set of random values
            newvals=0.0f;
            for (int mm=1; mm<=nin; mm++) {
                int binno=0;
                // look up cummulative dist and interp
                for (int midx=maxcidx; midx>=1; midx--) { if (cumnb(midx)>idx(mm)) binno=midx; }  // find least index
                if (binno==0) { binno=maxcidx; }
                float minv=borders(Max(1,binno-1));
                float maxv=borders(binno);
                newvals(mm)=(urand(0,100)/100.0f)*(maxv-minv) + minv;
            }

            if (verbose.value()) { cout << "Putting samples back into image" << endl; }
            if (verbose.value()) { cout << "Values are : " << newvals.t() <<  endl; }
            {
                volume4D<float> newinvol(invol);
                newinvol.setmatrix(newvals.t(),maskinr);  // put values back into image
                invol *= (1.0f - maskinr);
                invol += newinvol;
            }
        }
    }

    save_volume(invol[0],appendFSLfilename(outname.value(),"_inneronly"));

    // set the (inner) border of the lesion to a smoothed version of its neighbours (outside + filled interior)
    ColumnVector ker1d(3);
    ker1d << 0.7 << 1.0 << 0.7;
    volume<float> mask_inreg_outb, imn, maskn, maskbinv, imall, maskall, wall, wn;
    mask_inreg_outb = binarise(lesinreg + compb,0.5f);
    imn=convolve_separable(invol[0]*mask_inreg_outb,ker1d,ker1d,ker1d);
    maskn=convolve_separable(mask_inreg_outb,ker1d,ker1d,ker1d);
    maskbinv=binarise(1.0f-lesinb,0.5f);

    // simple alternative
    //invol[0]=invol[0]*maskbinv + lesinb*divide(imn,maskn,maskn);

    imall=convolve_separable(invol[0]*maskbinv,ker1d,ker1d,ker1d);
    maskall=convolve_separable(maskbinv,ker1d,ker1d,ker1d);

    //wall=0.5*(maskall>0) + 0.5*(maskall>0).*(maskn==0);
    //wn=0.5*(maskn>0) + 0.5*(maskn>0).*(maskall==0);
    float eps=1e-8;
    wall=0.5f*binarise(maskall,eps)*(binarise(maskn,-eps,eps)+1.0f);
    wn=0.5f*binarise(maskn,eps)*(binarise(maskall,-eps,eps)+1.0f);

    invol[0] = invol[0]*maskbinv + wall*lesinb*divide(imall,maskall,maskall) + wn*lesinb*divide(imn,maskn,maskn);

    save_volume(invol[0],outname.value());

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ORIGINAL MATLAB CODE

/*
masks=convn((1-lesinb),ones([3,3,3]),'same');
newinreg=(masks==0).*lesinb;
lesinb=lesinb-newinreg;
lesinreg=lesinreg+newinreg;

for compnum=1:maxcomp,

  disp(['  Component ',num2str(compnum)]);

  mask=(compb==compnum);
  maskinb=(comp==compnum).*lesinb;
  maskinr=(comp==compnum).*lesinreg;
  nin=sum3(maskinr);

  if (nin>0),
    vals=im(mask);
    nvox=length(vals);

    nb=1:5:nvox;
    sval=sort(vals);
    cumnb=cumsum(nb)/sum(nb);
    borders=sval(nb);

    disp('Made histogram');

    % sample from histogram for inner region
    idx=randperm(nin)/nin;  % make this truly random  - uniform [0,1]
    newvals=zeros([nin 1]);
    for mm=1:nin,
      binno=min(find(cumnb>idx(mm)));
      if (length(binno)==0),  binno=length(cumnb); end
      minv=borders(max(1,binno-1));
      maxv=borders(binno);
      newvals(mm)=rand(1)*(maxv-minv)+minv;
    end

    im(maskinr>0)=newvals;

    disp('Filled interior region');

  end
end

% average nearest neighbours of each voxel in the inner border

kernel=ones([3,3,3]);
k1d=[0.7 1 0.7];
kernel(:,:,1)=k1d(1)*k1d'*k1d;
kernel(:,:,2)=k1d(2)*k1d'*k1d;
kernel(:,:,3)=k1d(3)*k1d'*k1d;
disp('Averaging interior and boundary neighbourhood voxel for image');
imn=convn(im.*((lesinreg+compb)>0),kernel,'same');
disp('Averaging interior/boundary for mask');
maskn=convn(((lesinreg+compb)>0),kernel,'same');
disp('Filled in inner border');
%% CAN I REPLACE EVERYTHING BELOW WITH JUST: im=im.*(1-lesinb) + lesinb.*imn./max(0.00001,maskn)

disp('Averaging all neighbourhood voxels for image');
imall=convn(im.*(1-lesinb),kernel,'same');
disp('Averaging all neighbourhood for mask');
maskall=convn((1-lesinb),kernel,'same');
% calculate weights so that if both are valid it averages otherwise
% it takes just one of them
wall=0.5*(maskall>0) + 0.5*(maskall>0).*(maskn==0);
wn=0.5*(maskn>0) + 0.5*(maskn>0).*(maskall==0);
im=im.*(1-lesinb) + wall.*lesinb.*imall./max(0.00001,maskall) ...
   + wn.*lesinb.*imn./max(0.00001,maskn);


save_avw(im,outputname,'f',pixdims)

*/


////////////////////////////////////////////////////////////////////////////

int main(int argc,char *argv[])
{

    Tracer tr("main");
    OptionParser options(title, examples);

    try {
        // must include all wanted options here (the order determines how
        //  the help message is printed)
        options.add(involname);
        options.add(outname);
        options.add(lesmaskname);
        options.add(wmmaskname);
        options.add(verbose);
        options.add(writecomps);
        options.add(help);

        nonoptarg = options.parse_command_line(argc, argv);

        // line below stops the program if the help was requested or
        //  a compulsory option was not set
        if ( (help.value()) || (!options.check_compulsory_arguments(true)) )
        {
            options.usage();
            exit(EXIT_FAILURE);
        }

    }  catch(X_OptionError& e) {
        options.usage();
        cerr << endl << e.what() << endl;
        exit(EXIT_FAILURE);
    } catch(std::exception &e) {
        cerr << e.what() << endl;
    }

    // Call the local functions

    return do_work(argc,argv);
}
