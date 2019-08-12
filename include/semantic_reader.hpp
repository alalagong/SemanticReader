#ifndef _SEMANTIC_READER_HPP_
#define _SEMANTIC_READER_HPP_

#include <string>
#include <vector>
#include <map>
#include "dirent.h"
#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>
#include "maskApi.h"

namespace semantic_reader
{

struct Object
{
    cv::Rect box;
    cv::Mat mask;
};

enum LABEL { Road=0, Sidewalk, Building, Wall, Fence, Pole, Traffic_light,
            Traffic_sign, Vegetation, Terrain, Sky, Person, Rider, Car, Truck,
            Bus, Train, Motorcycle, Bicycle };

//!  Not initial, may occur some errors when null label??
struct Semantic
{
public:

    Object& getObject(LABEL label) {if(classes.find(label) == classes.end()) std::cout<<"There is't this label"<<std::endl; return classes[label]; }
    cv::Rect& getBox(LABEL label) {if(classes.find(label) == classes.end()) std::cout<<"There is't this label"<<std::endl; return classes[label].box; }
    cv::Mat& getMask(LABEL label) {if(classes.find(label) == classes.end()) std::cout<<"There is't this label"<<std::endl; return classes[label].mask; }
    cv::Size& getImgSize() { return img_size; }
    cv::Mat& getLabelImg() { return label_img; }
    cv::Mat& getConfImg() { return confidence_img;}
    size_t getLabelNum() {return class_nums;}

    std::map<LABEL, Object> classes;
    cv::Size img_size;
    cv::Mat label_img;
    cv::Mat confidence_img;
    size_t class_nums;       // numbers of classs of every img 
    double timestamp;  // TODO

};

// writting

inline bool isYAML(std::string &filename)
{
    int len = filename.length();
    if(filename.at(0) == '.')
        return false;
    
    if(filename.substr(len-5, 5 ) == ".yaml")
        return true;
    else
        return false;
}

inline bool is_base64(const char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

class SemanticReader
{
public:
    SemanticReader(std::string path): directory_(path), cur_id(-2)
    {
        DIR *dir;
        struct dirent *dirp;

        if((dir = opendir(directory_.c_str())) == NULL)
        {
            std::cout << " please check dir path " << std::endl;
        }

        // pick up yaml
        while( (dirp = readdir(dir)) != NULL )
        {
            std::string name = std::string(dirp->d_name);

            if( isYAML(name) )
                files_.push_back(name);
        }
        closedir(dir);

        files_nums_ = files_.size();

        std::sort(files_.begin(), files_.end());
        
        // add path
        if( directory_.at( directory_.length()-1) != '/' )
            directory_ = directory_ + "/";

        std::for_each(files_.begin(), files_.end(), [&](std::string &name){
            if(name.at(0) != '/') name = directory_ + name;
         });
    }

    Semantic& getSemantic( int img_id )
    {
        if( img_id == cur_id) // decode once
        {
            return sem;
        }

        if( img_id >= files_nums_)
        {
            std::cout << "out of files number "<< files_nums_<<std::endl;
            return sem_null;
        }

        cv::Mat confidence_img, label_img;
        std::map<LABEL, Object> classes;

        YAML::Node config = YAML::LoadFile( files_[img_id].c_str() );
        YAML::Node instance = config["instance"];       // instance
        std::string confid_string = config["confidence"].as<std::string>();  // confidence 
        
        confidence_img = base2Mat(confid_string);

        Object obj;
        YAML::Node box;
        YAML::Node segment;
        cv::Size img_size = confidence_img.size();

        for(int i=0; i<instance.size(); ++i)
        {
            int label_id = instance[i]["category_id"].as<int>();
            
            // all labels img
            if( label_id == -1)
            {
                box = instance[i]["box"];
                if(!(box[0].as<int>()==0 && box[1].as<int>()==0 && box[2].as<int>()==img_size.width && box[3].as<int>()==img_size.height))
                {
                    std::cout<<"size of labels image error in yaml"<<std::endl;
                    break;
                }
                std::string label_string = instance[i]["segmentation"].as<std::string>();
                label_img = base2Mat(label_string);
                continue;
            }

            box = instance[i]["box"];  // box
            segment = instance[i]["segmentation"];  // segmentation
            
            obj.box = cv::Rect(box[1].as<int>(), box[0].as<int>(), box[3].as<int>(), box[2].as<int>());
            obj.mask = cocoMaskDecode(segment, img_size);

            auto category = std::make_pair(LABEL(label_id), obj);
            classes.insert( category );
        }
        
        sem.classes = classes;
        sem.confidence_img = confidence_img;
        sem.class_nums = sem.classes.size();
        sem.label_img = label_img;
        sem.img_size = img_size;
        cur_id = img_id;

        return sem;
    }

    int getFileNum() {return files_nums_; }

private:
    std::string directory_;
    std::vector<std::string> files_;
    size_t files_nums_;    // number of img
    Semantic sem;
    Semantic sem_null;
    
    int cur_id;

    const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

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

    cv::Mat base2Mat(std::string &base64_data)
    {
    	cv::Mat img;
    	std::string s_mat;
        s_mat = base64_decode(base64_data);
    	std::vector<char> base64_img(s_mat.begin(), s_mat.end());
    	img = cv::imdecode(base64_img, CV_LOAD_IMAGE_COLOR);
    	return img;
    }

    cv::Mat cocoMaskDecode(YAML::Node &segment, cv::Size &img_size)
    {
        cv::Mat mask;
        // size
        cv::Size segment_size(segment["size"][1].as<int>(), segment["size"][0].as<int>());
        // check
        if(img_size != segment_size)
        {
            std::cout<< "Image size is "<<"("<<img_size.width<<", "<<img_size.height<<")"<<
                    "but mask is"<<"("<<segment_size.width<<", "<<segment_size.height<<")"<<std::endl;
            return mask;            
        }
        std::string segment_counts = segment["counts"].as<std::string>();
        
        unsigned char *mask_data = new uchar[segment_size.width*segment_size.height];
        COCOAPI::RLE *r = new COCOAPI::RLE();
        COCOAPI::rleFrString(r, segment_counts.c_str(), segment_size.height, segment_size.width);
        COCOAPI::rleDecode(r, mask_data, 1);
        COCOAPI::rleFree(r);  // rle decode complete
        mask = cv::Mat(segment_size.width, segment_size.height, CV_8UC1, mask_data);
        mask = mask*255;
        cv::transpose(mask, mask);
        return mask;
    }

};


} // semantic_reader

#endif // _SEMANTIC_READER_HPP_