#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "maskApi.h"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(const char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

 
std::string base64_decode(std::string const & encoded_string)
{
    int in_len = (int) encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);  
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];  

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];  
    }  

    return ret;  
}

 
static cv::Mat Base2Mat(std::string &base64_data)
{
	cv::Mat img;
	std::string s_mat;
    s_mat = base64_decode(base64_data);
	std::vector<char> base64_img(s_mat.begin(), s_mat.end());
	img = cv::imdecode(base64_img, CV_LOAD_IMAGE_COLOR);
	return img;
}

void readByOpencv(std::string file)
{
    cv::FileStorage fs(file.c_str(), cv::FileStorage::READ);

    if( !fs.isOpened() )
        return;
    // cv::FileNodeIterator fs1 = fs[0];
    cv::FileNode instance = fs["instance"];
    cv::FileNodeIterator it = instance.begin(), it_end = instance.end();
    int count = 0;
    for(; it != it_end; ++it)
    {
        if(count++ == 2)
            break;
        //! category
        std::cout<<"category_id" << (int)(*it)["category_id"]<<"\n";
        
        //! dbox (后面会改成box)
        cv::FileNode box_node = (*it)["dbox"];
        std::vector<int> bbox;
        for(int i = 0; i<box_node.size(); ++i)
        {
            bbox.push_back(box_node[i]);
        }
        std::cout<<"dbox:" <<bbox[0]<<" "<<bbox[1]<<" "<<bbox[2]<<" "<<bbox[3]<<std::endl;
        
        //! segment        
        cv::FileNode segment = (*it)["segmentation"];
        std::string seg = static_cast<std::string>(segment["counts"]);
        std::cout<<seg<<std::endl;
        // cv::FileNodeIterator it_seg = segment.begin();
        // for(; it_seg != segment.end(); ++it_seg)
        // {
        //     std::string a = (*it_seg)["a"] ;
        //     std::cout<< "segment::a: "<<a<<std::endl; 
        // }
        
    }

    std::string confidence = static_cast<std::string>(fs["confidence"]);
    std::vector<uchar> data(confidence.begin(), confidence.end());
    cv::Mat img_decode;
    img_decode = cv::imdecode(data, CV_LOAD_IMAGE_COLOR);
    cv::imshow("decode", img_decode);
    cv::waitKey(0);
}

void encodeImg(std::string file)
{
    cv::Mat img = cv::imread(file.c_str());
    std::vector<uchar> data_encode;
    cv::imencode(".png", img, data_encode);
    std::string str_encode(data_encode.begin(), data_encode.end());

    std::cout<<str_encode<<std::endl;
    std::cout<<std::endl;
}



void readByYamlcpp(std::string file)
{
     // 加载配置文件.
    YAML::Node config = YAML::LoadFile(file.c_str());

    // 节点类型.
    std::cout << "Node type " << config.Type() << std::endl;                      // 类型为 map.
    std::cout << "confidence type " << config["confidence"].Type() << std::endl;  // 类型为 Scalar.

    //! confidence
    // 定义一个 string 类型的变量存储编码信息.
    std::string code_string = config["confidence"].as<std::string>();

    //!instance
    YAML::Node instance = config["instance"];
    std::cout<< instance.size() <<std::endl;

    int id = instance[0]["category_id"].as<int>();
    
    YAML::Node box = instance[0]["box"];
    cv::Rect rect(box[0].as<int>(), box[1].as<int>(), box[2].as<int>(), box[3].as<int>());

    YAML::Node segment = instance[0]["segmentation"];
    cv::Size segment_size(segment["size"][1].as<int>(), segment["size"][0].as<int>());
    std::string segment_counts = segment["counts"].as<std::string>();
    std::cout << segment_counts << std::endl;

    unsigned char *mask_data = new uchar[segment_size.width*segment_size.height];
    COCOAPI::RLE *r = new COCOAPI::RLE();
    COCOAPI::rleFrString(r, segment_counts.c_str(), segment_size.height, segment_size.width);
    COCOAPI::rleDecode(r, mask_data, 1);
    COCOAPI::rleFree(r);  // rle decode complete
    cv::Mat mask = cv::Mat(segment_size.width, segment_size.height, CV_8UC1, mask_data);
    mask = mask*255;
    cv::transpose(mask, mask);

    cv::imshow("mask", mask);
    cv::waitKey(0);

    // 解码图像.
    cv::Mat encode_img = Base2Mat(code_string);
    if(encode_img.cols > 0)
    {
        imshow("picture", encode_img);
        cv::waitKey(0);
    }
 

}

int main()
{

    // encodeImg("/home/gong/Dataset/07/000029_bel.png");
    readByYamlcpp("/home/gong/Project/C++/read_yaml/data/000000.yaml");

}