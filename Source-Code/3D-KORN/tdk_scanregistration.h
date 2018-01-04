#ifndef TDK_SCANREGISTRATION_H
#define TDK_SCANREGISTRATION_H

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <math.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/common/transforms.h>
#include <pcl/features/normal_3d.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/point_types_conversion.h>
#include <pcl/registration/correspondence_estimation_backprojection.h>
#include <pcl/registration/correspondence_rejection_sample_consensus.h>
#include <pcl/registration/elch.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/incremental_registration.h>
#include <pcl/registration/transformation_estimation_svd.h>
#include <vector>
#include <QColor>
#include <QObject>
#include <QString>

#include "tdk_2dfeaturedetection.h"

using namespace std;

void PointCloudXYZRGBtoXYZ(
        const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &in,
        pcl::PointCloud<pcl::PointXYZ>::Ptr &out
        );
/*!
 * \brief tdk_PointCloudXYZRGBtoXYZI
 * \param in pointer for input point cloud
 * \param out output transformed point cloud
 *
 * Function transforms input XYZRGB (colored) point cloud into XYZI (intensity) point cloud
 */
void tdk_PointCloudXYZRGBtoXYZI(
        const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &in,
        pcl::PointCloud<pcl::PointXYZI>::Ptr &out
        );


class TDK_ScanRegistration: public QObject
{
    Q_OBJECT
public:
    TDK_ScanRegistration();
    TDK_ScanRegistration(const bool registerInRealTime);
    TDK_ScanRegistration(const pcl::PointWithViewpoint scannerCenter,
                         const bool registerInRealTime);
    ~TDK_ScanRegistration();

    //Input
    bool addNextPointCloud(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &inputPointcloud,
                           const float degreesRotatedY=0.0);

    //Ouput
    pcl::PointCloud<pcl::PointXYZ>::Ptr getLastDownSampledPointcloud();
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getRoughlyAlignedPC();
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr Process_and_getAlignedPC();
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>* getRotationCompensatedPCs();
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>* getRoughlyAlignedPCs();

    //Configuration parameters getters and setters
    bool getRegisterInRealTime() const;
    void setRegisterInRealTime(bool value);

    pcl::PointWithViewpoint getScannerRotationAxis() const;
    void setScannerRotationAxis(const pcl::PointWithViewpoint &value);

    float get_normalRadiusSearch() const;
    float get_ICPPost_MaxCorrespondanceDistance() const;
    float get_voxelSideLength() const;
    double get_SVD_MaxDistance() const;
    float get_ICP_MaxCorrespondenceDistance() const;

    void set_normalRadiusSearch(float value);
    void set_voxelSideLength(float value);
    void set_SVD_MaxDistance(double value);
    void set_ICP_MaxCorrespondenceDistance(float value);
    void set_PostICP_MaxCorrespondanceDistance(float value);

signals:
    void mf_SignalStatusChanged(QString, QColor);

public slots:
    void set_Use2DFeatureDetection(int);


private:
    //Class operation configuration
    bool mv_registerInRealTime;
    bool mv_use2DFeatureDetection = false;

    //Configuration parameters
    float mv_normalRadiusSearch;
    float mv_voxelSideLength;
    double mv_SVD_MaxDistance;
    float mv_ICP_MaxCorrespondenceDistance;
    float mv_ICPPost_MaxCorrespondanceDistance;

    //Scanner orientation and rotation compensation
    bool mv_scannerCenterRotationSet;
    pcl::PointWithViewpoint mv_scannerCenter;
    float mv_accumulatedRotation;

    //Internal data storage
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> mv_originalPCs;
    vector<float> mv_originalPointcloudsYRotation;
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> mv_originalRotatedPCs;
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> mv_originalRotatedDenoisedPCs;
    vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> mv_downSampledPCs;
    vector<pcl::PointCloud<pcl::Normal>::Ptr> mv_downSampledNormals;
    vector<pcl::CorrespondencesPtr> mv_downsampledCorrespondences;
    vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> mv_alignedDownSampledPCs;
    vector<Eigen::Matrix4f> mv_transformationMatrices;
    vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> mv_alignedOriginalPCs;

    //feature detection service
    TDK_2DFeatureDetection mv_2DFeatureDetectionPtr;

    //Private class functions
    bool
    mf_processCorrespondencesSVDICP();
    bool
    mf_processInPostWithICP();

    bool
    addAllPointClouds(const vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> &inputPCs,
                      const vector<float> degreesRotatedY);

    void
    setDefaultParameters();

    //Utility functions
    pcl::PointCloud<pcl::PointXYZ>::Ptr
    mf_outlierRemovalPC(const pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud_in,
                        const float meanK=8,
                        const float std_dev=2.5);

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr
    mf_outlierRemovalPC(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud_in,
                        const float meanK=8,
                        const float std_dev=2.5);

    pcl::PointCloud<pcl::PointXYZ>::Ptr
    mf_voxelDownSamplePointCloud(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud_in,
                                 const float &voxelSideLength);

    pcl::PointCloud<pcl::Normal>::Ptr
    mf_computeNormals(const pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud_in,
                      const float &searchRadius);

    pcl::CorrespondencesPtr
    mf_estimateCorrespondences(const pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud1,
                               const pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud2,
                               const pcl::PointCloud<pcl::Normal>::Ptr &normals1,
                               const pcl::PointCloud<pcl::Normal>::Ptr &normals2,
                               const double &max_distance);

    template <typename PointT>
    boost::shared_ptr<pcl::PointCloud<PointT>>
    mf_iterativeClosestPointFinalAlignment(const boost::shared_ptr<pcl::PointCloud<PointT>> &source,
                                           const boost::shared_ptr<pcl::PointCloud<PointT>> &target,
                                           const float &maxCorrespondenceDistance,
                                           Eigen::Matrix4f &icpTransformation);

    pcl::PointCloud<pcl::PointXYZ>::Ptr
    mf_SVDInitialAlignment(const pcl::PointCloud<pcl::PointXYZ>::Ptr &source,
                           const pcl::PointCloud<pcl::PointXYZ>::Ptr &target,
                           pcl::CorrespondencesPtr correspondences,
                           Eigen::Matrix4f &transformation_matrix);
};

#endif // TDK_SCANREGISTRATION_H
