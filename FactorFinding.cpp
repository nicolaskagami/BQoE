#include<stdio.h>
#include<vector>

#define CHUNK_SIZE 20000000 //b
#define CHUNK_DURATION 10 //Seconds
#define VIDEO_BUFFER_SIZE 20 //Seconds
#define CAPACITY 100000000 //Mb
#define TICKS_PER_SEC 100
#define MEASUREMENT_DURATION 10 //Seconds
#define EXPERIMENT_DURATION 3600 //Seconds


int active_amount = 0;
int stall_duration = 0;
class VideoInstance
{
    public:
    int duration; //ticks
    int current_time; //ticks
    int buffered; //ticks 
    int chunk_progress; //Mb
    bool active;
    bool done;

    void count_active() { if(active) active_amount++;};
    void run() 
    {
        if(active)
        {
            chunk_progress+=(CAPACITY/(active_amount*TICKS_PER_SEC));
            if(chunk_progress>=CHUNK_SIZE)
            {
                buffered+=(chunk_progress/CHUNK_SIZE)*CHUNK_DURATION*TICKS_PER_SEC;
                chunk_progress = chunk_progress%CHUNK_SIZE;
            }
        }

        if((buffered>0)&&(current_time<duration))
        {
            //printf("Time: %d +%d (%d)\n",current_time, buffered, duration);
            current_time++;
            buffered--;
        }
        else
        {
            stall_duration++;
        }

        if(current_time>=duration)
            done = true;

        if((buffered/TICKS_PER_SEC)>=VIDEO_BUFFER_SIZE)
            active = false;
        else
            active = true;
    };

    VideoInstance(int Duration)
    {
        active = true;
        chunk_progress = 0;
        duration = Duration*TICKS_PER_SEC;
        current_time = 0;
        buffered = 0;
        done = false;
    };
    ~VideoInstance(){};
};
int tick;
int measurement_ratio = 6;
int main(int argc, char ** argv)
{
    int measured;
    float acquired_ratio_cumulative = 0;
    int number_of_measurements =0;
    std::vector<VideoInstance> videos;
    for(int i = 0; i< 10;i++)
        videos.push_back(VideoInstance(3600));

    for(tick=0;tick<EXPERIMENT_DURATION*TICKS_PER_SEC;tick++)
    {
        active_amount =0;
        for (std::vector<VideoInstance>::iterator it = videos.begin() ; it != videos.end(); ++it)
            it->count_active();
        if(!((tick/(MEASUREMENT_DURATION*TICKS_PER_SEC))%measurement_ratio))
        {
            if(!(tick%(measurement_ratio*MEASUREMENT_DURATION*TICKS_PER_SEC)))
                measured = 0;
            active_amount++;
            measured+=CAPACITY/(active_amount*TICKS_PER_SEC);
            if(!((tick+1-(MEASUREMENT_DURATION*TICKS_PER_SEC))%(measurement_ratio*MEASUREMENT_DURATION*TICKS_PER_SEC)))
            {
                if(videos.size())
                {
                    acquired_ratio_cumulative+=(float) (videos.size()*(CHUNK_SIZE/CHUNK_DURATION))/(measured/MEASUREMENT_DURATION);
                    number_of_measurements++;
                    printf("Measured: %d bps. Videos: %d\n",measured/MEASUREMENT_DURATION,videos.size());
                }
            }
        }
        for (std::vector<VideoInstance>::iterator it = videos.begin() ; it != videos.end(); ++it)
            it->run();
        for (std::vector<VideoInstance>::iterator it = videos.end() ; it != videos.begin(); --it)
            if(it->done)
                videos.erase(it);
        //if((active_amount<10)&&(!(tick%100)))
        //{getchar();
        //    printf("tick:%d\n",tick);
        //    printf("Videos: %d/%d\n",active_amount,videos.size());
        //}
    }
    printf("Total Stall Duration: %d seconds\n",stall_duration/TICKS_PER_SEC);
    printf("Acquired Ratio: %.2f\n",(float)acquired_ratio_cumulative/number_of_measurements);
}
