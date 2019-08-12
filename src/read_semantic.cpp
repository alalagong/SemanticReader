
#include "semantic_reader.hpp"
#include <chrono>

typedef std::chrono::steady_clock timer;

int main(int argc, char** argv)
{
    if(argc != 2 )
        std::cout<<"*** please input path to files ***"<<std::endl;

    // opendir
    semantic_reader::SemanticReader reader(argv[1]);
    std::cout<< reader.getFileNum() << "yalms"<<std::endl;
    
    std::chrono::time_point<timer> t1 = timer::now();
    
    cv::Rect box0 = reader.getSemantic(0).getBox(semantic_reader::Building);
    cv::Mat mask0 = reader.getSemantic(0).getMask(semantic_reader::Building);
    cv::Mat label_img = reader.getSemantic(0).getLabelImg();
    cv::Mat conf_img = reader.getSemantic(0).getConfImg();
    int label_num = reader.getSemantic(0).getLabelNum();
    cv::Size size0 = reader.getSemantic(0).getImgSize();

    std::chrono::time_point<timer> t2 = timer::now();
    int last = std::chrono::duration_cast<std::chrono::microseconds> (t2-t1).count();

    std::cout << "one cost "<<last<<" microseconds"<<std::endl;
    
    cv::Mat img = label_img*10;
    cv::imshow("mask_build", mask0);
    cv::imshow("label", img);
    cv::imshow("confidence", conf_img);

    std::cout<<"img size: "<<"("<<size0.width<<","<<size0.height<<")"<<std::endl;
    std::cout<<"Have "<< label_num<<" classes"<<std::endl;

    cv::rectangle(img, box0, cv::Scalar(255));
    cv::imshow("build_box", img);

    cv::waitKey(0);
}