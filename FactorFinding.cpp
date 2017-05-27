#include<stdio.h>
#include<vector>

#define CHUNK_SIZE 20000000 //b
#define CHUNK_DURATION 10 //Seconds
#define VIDEO_BUFFER_SIZE 120 //Seconds
#define CAPACITY 800000000 //Mb
#define TICKS_PER_SEC 100
#define MEASUREMENT_DURATION 10 //Seconds
#define EXPERIMENT_DURATION 10800 //Seconds


long active_amount = 0;
long stall_duration = 0;
long throughput = 0;
long tick;
long measurement_ratio = 6;

class VideoInstance
{
    public:
    long duration; //ticks
    long current_time; //ticks
    long buffered; //ticks 
    long chunk_progress; //Mb
    bool active;
    bool done;

    void count_active() { if(active) active_amount++;};
    void run() 
    {
        if(active)
        {
            chunk_progress+=(CAPACITY/(active_amount*TICKS_PER_SEC));
            throughput+=(CAPACITY/(active_amount*TICKS_PER_SEC));
            if(chunk_progress>=CHUNK_SIZE)
            {
                if(buffered>(2*VIDEO_BUFFER_SIZE*TICKS_PER_SEC))
                    printf("%d\n",buffered);
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

    VideoInstance(long Duration)
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
int main(int argc, char ** argv)
{
    long measured;
    float acquired_ratio_cumulative = 0;
    long number_of_measurements =0;
    std::vector<VideoInstance> videos;

    for(tick=0;tick<EXPERIMENT_DURATION*TICKS_PER_SEC;tick++)
    {
        if(!(tick%(21*TICKS_PER_SEC)))
            for(long i = 0; i< 10;i++)
                videos.push_back(VideoInstance(300));
        if(!(tick%(81*TICKS_PER_SEC)))
            for(long i = 0; i< 10;i++)
                videos.push_back(VideoInstance(1200));
        if(!(tick%(321*TICKS_PER_SEC)))
            for(long i = 0; i< 10;i++)
                videos.push_back(VideoInstance(2400));
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
                //float ratio = (float) (measured/MEASUREMENT_DURATION)/(CAPACITY-(videos.size()*(CHUNK_SIZE/CHUNK_DURATION)));
                long goodput = throughput/(MEASUREMENT_DURATION*measurement_ratio);
                float ratio = (float) (measured/MEASUREMENT_DURATION)/(CAPACITY-goodput);
                acquired_ratio_cumulative+=ratio;
                number_of_measurements++;
                //printf("Throughput: %ld\n",throughput/(MEASUREMENT_DURATION*measurement_ratio));
                throughput = 0;
                printf("Ratio: %f \tMeasured: %10ld bps \tActual: %10ld \t%.2f%\n", ratio,measured/MEASUREMENT_DURATION,(CAPACITY-goodput),(float)(100)*(CAPACITY-goodput)/CAPACITY);
            }
        }
        for (std::vector<VideoInstance>::iterator it = videos.begin() ; it != videos.end(); ++it)
            it->run();
        for (std::vector<VideoInstance>::iterator it = videos.end() ; it != videos.begin(); --it)
            if(it->done)
                videos.erase(it);
    }
    printf("Total Stall Duration: %d seconds\n",stall_duration/TICKS_PER_SEC);
    printf("Acquired Ratio: %.2f\n",(float)acquired_ratio_cumulative/number_of_measurements);
}
