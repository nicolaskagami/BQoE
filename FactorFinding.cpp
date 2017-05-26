#include<stdio.h>
#include<vector>

#define CHUNK_SIZE 20 //Mb
#define CHUNK_DURATION 10 //Seconds
#define VIDEO_BUFFER_SIZE 60 //Seconds
#define CAPACITY 100 //Mb


int active_amount = 0;
class VideoInstance
{
    public:
    int duration;
    int current_time;
    int buffered;
    float chunk_progress;
    bool active;
    bool done;

    void count_active() { if(active) active_amount++;};
    void run() 
    {
        if(active)
        {
            chunk_progress+=CAPACITY/active_amount;
            if(chunk_progress>=CHUNK_SIZE)
            {
                buffered+=(int)(chunk_progress/CHUNK_SIZE)*CHUNK_DURATION;
                chunk_progress = 0;
            }
        }

        if((buffered>0)&&(current_time<duration))
        {
            current_time++;
            buffered--;
        }

        if(current_time>=duration)
            done = true;

        if(buffered>=VIDEO_BUFFER_SIZE)
            active = false;
        else
            active = true;
    };

    VideoInstance(int Duration)
    {
        active = true;
        chunk_progress = 0;
        duration = Duration;
        current_time = 0;
        buffered = 0;
        done = false;
    };
    ~VideoInstance(){};
};
int tick;
int main(int argc, char ** argv)
{
    std::vector<VideoInstance> videos;
    for(int i = 0; i< 10;i++)
    {
        videos.push_back(VideoInstance(10));
    }
    for(tick=0;tick<100;tick++)
    {
        active_amount =0;
        for (std::vector<VideoInstance>::iterator it = videos.begin() ; it != videos.end(); ++it)
            it->count_active();
        for (std::vector<VideoInstance>::iterator it = videos.begin() ; it != videos.end(); ++it)
            it->run();
        for (std::vector<VideoInstance>::iterator it = videos.end() ; it != videos.begin(); --it)
            if(it->done)
                videos.erase(it);
        printf("Videos: %d/%d\n",active_amount,videos.size());
        
    }
        
}
