#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

#include <string>
#include "dirent.h"
#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace arma;

const string featureConfig = "-hesaff -sift -noangle";
const string computeDescriptorPath = "compute_descriptors.exe";

mat inv2x2(mat C) {
    mat den = C.row(0) % C.row(2) - C.row(1) % C.row(1);
    mat S = join_vert(join_vert(C.row(2), - C.row(1)), C.row(0)) / repmat(den.row(0), 3, 1);
    return S;
}

bool vl_ubcread(string file, mat &f, umat &d) {
    int numKeypoints, descLen;
    FILE *fid = fopen(file.c_str(), "r");

    if (fid == NULL)
        return false;

    fscanf(fid, "%d %d", &descLen, &numKeypoints);

    f = mat(5, numKeypoints);

    d = umat(descLen, numKeypoints);

    for (int k = 0; k < numKeypoints; ++k) {
        fscanf(fid, "%lf %lf %lf %lf %lf", &f(0, k), &f(1, k), &f(2, k), &f(3, k), &f(4, k));
        for (int i = 0; i < descLen; ++i)
            fscanf(fid, "%d", &d(i, k));
    }
    fclose(fid);

    f.rows(0, 1) = f.rows(0, 1) + 1;
    f.rows(2, 4) = inv2x2(f.rows(2, 4));

    return 1;
}

void convertJPGtoPNG(string filename) {
    cv::Mat im = cv::imread(filename);
    cv::imwrite(filename.replace(filename.size() - 3, 3, "png"), im);
}

void extractFeatures(string imagePath, mat &kpMat, mat &siftMat) {
    string tempFile = "./temp.mat";

    string cmd = computeDescriptorPath + " " + featureConfig + " -i "
        + imagePath + " -o1 " + tempFile;

    system(cmd.c_str());

    mat clip_kp;
    umat clip_desc;

    if (!vl_ubcread(tempFile, clip_kp, clip_desc)) {
        clip_kp = mat(5, 0);
        clip_desc = umat(128, 0);
    //                cout << "k" << endl;
    }

    mat sift = conv_to<mat>::from(clip_desc);

    mat sqrt_desc = sqrt(sift / repmat(sum(sift), 128, 1));

    kpMat = clip_kp;
    siftMat = sqrt_desc;
}

#endif