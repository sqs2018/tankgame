#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include <SDL/SDL.h>		//-lSDL
#include <SDL/SDL_image.h>	//-lSDL_image
#include <SDL/SDL_ttf.h>	//-lSDL_ttf.h
#include <SDL/SDL_mixer.h>	//-lSDL_mixer.h


//默认窗口大小
#define     SCREEN_WIDTH	320
#define     SCREEN_HEIGHT   560
//分辨率
#define     BPP             32


#define     MAX_BULLET      6//坦克最多可以发射子弹
#define     PLAN_MAX_BULLET 5 //飞机最多可以射子弹
#define     MAX_SMALL_ENEMY	2  //小飞机最多多少架
#define     MAX_MID_ENEMY   1  //中型飞机最多多少架


//游戏状态
enum {
    GAME_START,
    GAME_MENU,
    GAME_RUNNING,
    GAME_OVER,
    GAME_EXIT,
};
//界面使用到的SDL_Surface
typedef struct GameEngine
{
    SDL_Surface *window;
    SDL_Surface	*background;
    SDL_Surface	*picture;
    SDL_Surface	*begin;
    SDL_Surface	*score;
}GAME_ENGINE;

//坦克、飞机结构
typedef struct plane
{
    SDL_Rect src_rect[10];  //保存中枪以及正常时候的图片信息
    SDL_Rect dst_rect;
    int pic_num;		//图片数量，从1开始
    int pic_index;		//图片索引，用于查找图片，从0开始
    int life;			//表示右几条命，0代表死亡
    int speed_x;		//飞机移动x方向的速度
    int speed_y;		//飞机移动y方向的速度
}PLANE;
//SDL的开始
void beginSDL(int flag);
//SDL的结束
void endSDL(int flag);
//游戏的菜单
void game_menu();
//游戏的运行
void game_running();
//定时器
void timer(int num);
//显示分数
void show_score();

//初始化每个部分的坐标，不同状态显示效果不一样，
//例如子弹被射击之后，显示多长爆炸效果
void init_game();
//初始化SDL
void init_SDL();
//初始化随机数
void init_rand();
//初始化定时器
void init_timer();
//初始化背影
void init_begin_background(PLANE *background);
//背景色
void init_background(PLANE *background);
//初始化坦克
void init_player(PLANE *player);
//初始化子弹
void init_bullet(PLANE *bullet);
//初始化小飞机
void init_small_enemy(PLANE *small_enemy);
//初始化中飞机
void init_mid_enemy(PLANE *mid_enemy);
//初始化从图上获取的数字图片的坐标
void init_number(PLANE *number);
//坦克发射子弹
void shot_bullet(PLANE *player, PLANE *bullet);
//飞机发射子弹
void shot_plan_bullet(PLANE *player,PLANE *bullet);
//显示元素
void show_one_element(GAME_ENGINE *game_engine, PLANE *element, int i);
//显示所有元素
void show_all_element(GAME_ENGINE *game_engine, PLANE *element, int max_element);

//检测飞机是否到边框
void check_edge(PLANE *player);

//从新给敌人新的坐标和命
//enemy:敌人
//life:生命
void get_enemy_pos(PLANE *enemy, int life);
int  check_shot_enemy(PLANE *enemy, int max_enemy, PLANE *bullet, int max_bullet );
//检查子弹有没有打到坦克上
int check_my();
void move_bullet(PLANE *bullet);
void move_plan_bullet(PLANE *bullet);
void show_little_plane();
SDL_Surface *load_image(SDL_Surface *image, char *file);

/**上面是plan.h**/

int count = 0;
//背景移动的速度
int count_1 = 0;
//刷新显示
int count_2 = 0;



/*****plan.c*******/

//坦克每个子弹对应一个结构体变量
PLANE bullet[MAX_BULLET] = {0};


//飞机每个子弹对应一个结构体变量
PLANE planbullet[PLAN_MAX_BULLET];


//每个小型飞机对应一个结构体变量
PLANE small_enemy[MAX_SMALL_ENEMY] = {0};
//每个中型飞机对应一个结构体变量
PLANE mid_enemy[MAX_MID_ENEMY] = {0};
//开始界面的小飞机
PLANE little_plane = {0};
//保存玩家信息
PLANE player;
//保存背景信息
PLANE background;
//保存开始背景的信息
PLANE begin_background;
//存数字的图片的数组
PLANE number[10] = {0};
//保存生命
int score_record = 1000;
//将分数转成字符串之后保存在这个数组里
//预留了8位
char score_arr[8] = {0};
//保存分数字符串的长度
int score_length = 0;
//统计坦克在屏幕中子弹的个数
int bullet_num = 0;

//统计飞机在屏幕中子弹的个数
int plan_bullet_num=0;

//一些变量保存在结构体中
GAME_ENGINE game_engine;
//事件结构体
SDL_Event event;
int game_state = 0;



//font
//SDL_Event event ;

int main(int argc, char *argv[])
{
    while(game_state != GAME_EXIT)
    {
        switch (game_state)
        {
            case GAME_START:
                init_game();
                game_state = GAME_MENU;
                break;
            case GAME_MENU:
                game_menu();
                break;
            case GAME_RUNNING:
                game_running();
                break;
        }
        printf("exit\n");
    }
    endSDL(SDL_INIT_VIDEO);
}






void show_score()
{
    int i, j;
    //将整形转为字符串存到数组中,并得到字符串的长度
    score_length = sprintf(score_arr, "%d", score_record);
    //判断字符串中每个元素的哪个数字并进行输出
    for(i = 0; i < score_length; i++)
    {
        j = *(score_arr+i)-'0';
        //在屏幕显示的坐标移动
        number[j].dst_rect.x += 16*i;
        //显示
        SDL_BlitSurface(game_engine.score, &(number[j].src_rect[0]), game_engine.window, &(number[j].dst_rect));

        //坐标清零，方便下次移动
        number[j].dst_rect.x = 0;
    }

    //死了
    if(score_record<0){

        //生命减少
        player.life -= 1;
        //图片索引增加
        player.pic_index++;

    }



}

//开始SDL
void beginSDL(int flag)
{
    //检查SDL是否运行，返回非0为已经运行
    if(SDL_WasInit(flag) != 0)
    {
        fprintf(stderr, "SDL is running!\n");
        return;
    }
    //转载SDL库，返回－1为失败
    if(SDL_Init(flag) == -1)
    {
        fprintf(stderr, "unable to init SDL!\n");
        return;
    }
    return;
}
//关闭SDL
void endSDL(int flag)
{
    //检查SDL是否运行，返回非0为已经运行
    if(SDL_WasInit(flag) == 0)
    {
        fprintf(stderr, "SDL is not running!\n");
        return;
    }
    //退出
    SDL_Quit();
    return;
}

//加载图片
SDL_Surface *load_image(SDL_Surface *image, char *file)
{
    image = IMG_Load(file);
    if(NULL == image)
    {
        perror("load picture fail\n");
        return NULL;
    }
    //除去截取到的图片周围的空白
    SDL_SetColorKey (image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, 255, 255, 255));
    //按窗口对象重新格式化输出
    image = SDL_DisplayFormat(image);
    return image;
}




void shot_plan_bullet(PLANE *player, PLANE *planbullet){
    int i, j;
    plan_bullet_num++;
    if(plan_bullet_num <= PLAN_MAX_BULLET)
    {
        for(i = 0; i < PLAN_MAX_BULLET; i++)
        {
            if(planbullet[i].life == 0)
            {
                planbullet[i].life = 1;
                planbullet[i].dst_rect.x = player->dst_rect.x +28;
                planbullet[i].dst_rect.y = player->dst_rect.y +28;
                break;
            }
        }
    }
    else if(plan_bullet_num > PLAN_MAX_BULLET)
        plan_bullet_num = PLAN_MAX_BULLET;
    else if(plan_bullet_num < 1)
        plan_bullet_num = 0;

}

//发射子弹
void shot_bullet(PLANE *player, PLANE *bullet)
{
    int i, j;
    bullet_num++;
    if(bullet_num <= MAX_BULLET)
    {
        for(i = 0; i < MAX_BULLET; i++)
        {
            if(bullet[i].life == 0)
            {
                bullet[i].life = 1;
                bullet[i].dst_rect.x = player->dst_rect.x + 28;
                bullet[i].dst_rect.y = player->dst_rect.y - 20;
                break;
            }
        }
    }
    else if(bullet_num > MAX_BULLET)
        bullet_num = MAX_BULLET;
    else if(bullet_num < 1)
        bullet_num = 0;
}
//显示一个敌人
void show_one_element(GAME_ENGINE *game_engine, PLANE *element, int i)
{
        SDL_BlitSurface(game_engine->picture, &(element[i].src_rect[element[i].pic_index]), game_engine->window, &(element[i].dst_rect));
}
//显示多个敌人
void show_all_element(GAME_ENGINE *game_engine, PLANE *element, int max_element)
{
    int i;
    for(i = 0; i < max_element; i++)
    {
        show_one_element(game_engine, element, i);
    }
}
//检查边缘
void check_edge(PLANE *player)
{
    if(player->dst_rect.x < 0)
        player->dst_rect.x = 0;
    else if(player->dst_rect.x > (SCREEN_WIDTH - player->src_rect[0].w))
        player->dst_rect.x = (SCREEN_WIDTH - player->src_rect[0].w);
    else if(player->dst_rect.y < 0)
        player->dst_rect.y = 0;
    else if(player->dst_rect.y > (SCREEN_HEIGHT - player->src_rect[0].h))
        player->dst_rect.y = SCREEN_HEIGHT - player->src_rect[0].h;
}
//从新给敌人新的坐标和命
void get_enemy_pos(PLANE *enemy, int life)
{
    int i = 0;
    //死亡，显示完爆炸效果之后，即图片索引为0
    //从新给生命，坐标
    while(enemy->life == 0 && enemy->pic_index == 0)
    {
        enemy->life = life;
        enemy->dst_rect.x = rand()%(SCREEN_WIDTH - enemy->src_rect[0].w);
        enemy->dst_rect.y = rand()%(SCREEN_WIDTH - enemy->src_rect[0].h*2);
        //防止随机坐标太靠边上
        if(enemy->dst_rect.x < 10 || enemy->dst_rect.x > (SCREEN_WIDTH-enemy->src_rect[0].w-10))
            continue;
        //enemy->dst_rect.y = 2;
    }
}
//检查坦克有没有被子弹打中
int check_my(){
    printf("player x=%d",player.src_rect->x);
    int i;
    for(i=0;i<PLAN_MAX_BULLET;i++){

        if(planbullet[i].life == 1){
            if(player.life==1){

                //检测子弹是否击中坦克
                if(planbullet[i].dst_rect.x>player.dst_rect.x&&planbullet[i].dst_rect.x<player.dst_rect.x+player.dst_rect.w){

                     if(planbullet[i].dst_rect.y>player.dst_rect.y&&planbullet[i].dst_rect.y<player.dst_rect.y+player.dst_rect.h){
                        //
                         score_record-=5;
                     }
                }
            }


        }


    }





}
//检查子弹是否射击到敌人
int check_shot_enemy(PLANE *enemy,  int max_enemy, PLANE *bullet, int max_bullet)
{
    int i, j;
    for(i = 0; i < max_bullet; i++)
    {
        for(j = 0; j < max_enemy; j++)
        {
            if(bullet[i].life == 1 && enemy[j].life > 0 && enemy[i].pic_index == 0)
            {
                //x方向判断
                if((enemy[j].dst_rect.y+enemy[j].src_rect[0].h) > bullet[i].dst_rect.y)
                {
                    //y方向判断
                    if( (bullet[i].dst_rect.x < (enemy[j].dst_rect.x+enemy[j].src_rect[0].w) && ((bullet[i].dst_rect.x+bullet[i].src_rect[0].w) > enemy[j].dst_rect.x)))
                    {
                        bullet_num -= 1;
                        bullet[i].life -= 1;
                        enemy[j].pic_index++;
                        enemy[j].life -= 1;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}



void show_little_plane()
{
    //开始界面，小飞机的动态效果
    SDL_BlitSurface(game_engine.begin, &begin_background.src_rect[0], game_engine.window, &begin_background.dst_rect);
    //刷新显示
    count_2++;
    if(count_2 == 60)
    {
        little_plane.src_rect[0].x = 86;
        little_plane.src_rect[0].y = 710;
        little_plane.src_rect[0].w = 40;
        little_plane.src_rect[0].h = 26;
        little_plane.dst_rect.x = 100;
        little_plane.dst_rect.y = 280;
    }
    else if(count_2 == 120)
    {
        little_plane.src_rect[0].x = 48;
        little_plane.src_rect[0].y = 710;
        little_plane.src_rect[0].w = 80;
        little_plane.src_rect[0].h = 26;
        little_plane.dst_rect.x = 100;
        little_plane.dst_rect.y = 280;
    }
    else if(count_2 == 180)
    {
        little_plane.src_rect[0].x = 0;
        little_plane.src_rect[0].y = 685;
        little_plane.src_rect[0].w = 122;
        little_plane.src_rect[0].h = 26;
        little_plane.dst_rect.x = 100;
        little_plane.dst_rect.y = 280;
    }
    //0-100不显示
    else if(count_2 == 240)
    {
        count_2 = 0;
    }
    if(count_2 > 60)
        SDL_BlitSurface(game_engine.picture, &little_plane.src_rect[0], game_engine.window, &little_plane.dst_rect);
    SDL_Flip(game_engine.window);

}

/****timer****/
int time_count = 0;


void timer(int num)
{
    //不能在定时器里刷新屏幕，频率太高会显示不出来
    if(game_state == GAME_RUNNING)
    {

        time_count++;		//用于控制运动的速度
        //背景移动
        count_1++;
        if(count_1 == 4)
        {
            count_1 = 0;
            background.src_rect[0].y--;
            if(background.src_rect[0].y == 0)
                background.src_rect[0].y = 560;
        }

        //飞机移动，通过按键控制固定的速度
        player.dst_rect.y += player.speed_y;
        player.dst_rect.x += player.speed_x;
        //检查飞机是否碰到边缘
        check_edge(&player);
        //子弹运动
        move_bullet(bullet);
       if(time_count%5 == 0){
         //飞机子弹运动
         move_plan_bullet(planbullet);
       }


        //飞机自动向右运动
        int i, j;
        if(time_count%2 == 0)
        {
            for(i = 0; i < MAX_SMALL_ENEMY; i++)
            {
                if(small_enemy[i].life > 0 && small_enemy[i].pic_index == 0)
                {
                    small_enemy[i].dst_rect.x += 2;


                    if(time_count%100 == 0){
                        //飞机自动发射子弹
                        //shot_bullet(&player,bullet);
                        shot_plan_bullet(&small_enemy[i],planbullet);
                    }


                }
                //敌人死亡之后，显示爆炸效果的图片
                else if(small_enemy[i].life == 0 && small_enemy[i].pic_index != 0)
                {
                        if(time_count%4 == 0)
                        {
                            //显示被射击之后的图像效果
                            small_enemy[i].pic_index++;
                            //图像效果显示完之后，消失
                            if(small_enemy[i].pic_index == small_enemy[i].pic_num)
                            {
                                small_enemy[i].pic_index = 0;
                                //分数增加
                                score_record += 100;
                            }
                        }
                }
                //到底端消失
                if(small_enemy[i].dst_rect.x  > SCREEN_WIDTH)
                {
                    small_enemy[i].life = 0;
                }
            }
        }
        //---------------中型飞机---------------
        if(time_count%4 == 0)
        {
            for(i = 0; i < MAX_MID_ENEMY; i++)
            {
                if(mid_enemy[i].life > 0 && mid_enemy[i].pic_index == 0)
                {
                    if(time_count%100 == 0){
                        //飞机自动发射子弹

                        shot_plan_bullet(&mid_enemy[i],planbullet);
                    }
                    mid_enemy[i].dst_rect.x += 2;
                }
                else if(mid_enemy[i].life >= 0 && mid_enemy[i].pic_index != 0)
                {
                        if(time_count%8 == 0)
                        {
                            //显示被射击之后的图像效果
                            if(mid_enemy[i].pic_index == 1 && mid_enemy[i].life > 0)
                            {
                                mid_enemy[i].pic_index = 0;
                                mid_enemy[i].dst_rect.x += 2;
                            }
                            else if(mid_enemy[i].pic_index >= 1 && mid_enemy[i].life == 0)
                                mid_enemy[i].pic_index++;
                            //图像效果显示完之后，消失
                            if(mid_enemy[i].pic_index == mid_enemy[i].pic_num)
                            {
                                mid_enemy[i].pic_index = 0;
                                score_record += 300;
                            }
                        }
                }
                //到底端消失
                if(mid_enemy[i].dst_rect.x > SCREEN_WIDTH)
                {
                    mid_enemy[i].life = 0;
                }
            }
        }

        //将分数转化为字符串,并进行显示
        show_score();

        //玩家的飞机
        if(time_count%2 == 0)
        {
            //死亡后显示爆炸效果
            if(player.life == 0 && player.pic_index != 0)
            {
                if(time_count%4 == 0)
                {
                    //显示被射击之后的图像效果
                    player.pic_index++;
                    //图像效果显示完之后，消失
                }
            }
            //到底端消失
            if(player.dst_rect.y > (SCREEN_HEIGHT-player.src_rect[0].h))
            {
                player.life = 0;
                player.dst_rect.y = 0;
            }
        }
        //飞机飞行动态
        if(time_count == 10)
        {
            player.src_rect[0].x = 433;
            player.src_rect[0].y = 331;
        }
        else if(time_count > 50 && time_count < 100)
        {
            player.src_rect[0].x = 433;
            player.src_rect[0].y = 0;
        }
        else if(time_count == 100)
            time_count = 0;
    }
}

void set_timer(int ms)
{
    struct itimerval val;

    val.it_interval.tv_sec = ms/2000;
    val.it_interval.tv_usec = ms%1000*2000;
    val.it_value.tv_sec = ms/2000;
    val.it_value.tv_usec = ms%1000*2000;

    setitimer(ITIMER_REAL, &val, NULL);
}

void move_bullet(PLANE *bullet)
{
    int i, j;
    for(i = 0; i < MAX_BULLET; i++)
    {
        if(bullet[i].life == 1)
        {
            //子弹向上移动5个像素点
            bullet[i].dst_rect.y -= 4;
            //子弹到到顶端消失
            if(bullet[i].dst_rect.y < 8)
            {
                //子弹生命变为0
                bullet[i].life = 0;
                //子弹数量减1
                bullet_num -= 1;
                continue;
            }
            //子弹生命为1才进行显示
            show_one_element(&game_engine, bullet, i);
        }
    }
}

void move_plan_bullet(PLANE *planbullet)
{
    int i, j;
    for(i = 0; i < PLAN_MAX_BULLET; i++)
    {
        if(planbullet[i].life == 1)
        {
            //子弹向上移动5个像素点
            planbullet[i].dst_rect.y += 4;
            //子弹到到diduan端消失
            if(planbullet[i].dst_rect.y >SCREEN_HEIGHT)
            {
                //子弹生命变为0
                planbullet[i].life = 0;
                //子弹数量减1
                plan_bullet_num -= 1;
                continue;
            }
            //子弹生命为1才进行显示
            show_one_element(&game_engine, planbullet, i);
        }
    }
}



/*game.c**/

void game_menu()
{
    while(game_state == GAME_MENU)
    {
        show_little_plane();
        if(SDL_PollEvent( &event))
        {
            printf("game menu\n");
            //按键事件检测，按下时，更新相应方向的速度
            if(event.type == SDL_KEYDOWN)
            {
                printf("keyevent\n");
        //===============ENTER进入游戏===============
                if(event.key.keysym.sym == SDLK_RETURN)
                {
                    //分数清零
                    score_record = 1000;
                    //开始标志
                    game_state = GAME_RUNNING;
                }
        //=================退出游戏====================
                if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    game_state = GAME_EXIT;
                }
            }
        //=============鼠标关系游戏窗口=================
            if(event.type == SDL_QUIT )
                game_state = GAME_EXIT;
        }
        //防止程序一直在检测事件
        SDL_Delay(10);
    }
}

void game_running()
{
    while(game_state == GAME_RUNNING)
    {
//============开始游戏==============
        printf("game_running\n");
        //显示背景
        SDL_BlitSurface(game_engine.background, &background.src_rect[0], game_engine.window, &background.dst_rect);

        //-------------按键--------------
        if(SDL_PollEvent( &event))
        {

            //按键事件检测，按下时，更新相应方向的速度
            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_w: player.speed_y = -2;break;
                    case SDLK_s: player.speed_y = 2;break;
                    case SDLK_a: player.speed_x = -2;break;
                    case SDLK_d: player.speed_x = 2;break;
                    case SDLK_j:		//手动射击
                              shot_bullet(&player, bullet);
                              //shot_plan_bullet(&small_enemy[0],planbullet);
                             break;
                }
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    game_state = GAME_START;
                    //begin_flag = 0;
            }
            //按键松开时，对速度清零
            else if(event.type == SDL_KEYUP)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_w: player.speed_y = 0;break;
                    case SDLK_s: player.speed_y = 0;break;
                    case SDLK_a: player.speed_x = 0;break;
                    case SDLK_d: player.speed_x = 0;break;
                }
            }
        }
        //延时一会
        SDL_Delay(10);
        //鼠标关闭右下窗口，退出游戏
        if(event.type == SDL_QUIT )
            break;
        //--------检查子弹是否射击到敌人----------------
        check_shot_enemy(small_enemy, MAX_SMALL_ENEMY, bullet, MAX_BULLET);
        check_shot_enemy(mid_enemy, MAX_MID_ENEMY, bullet, MAX_BULLET);
        //-------------飞机死亡判断--------------
        check_my();



        //飞机死亡回到上面的开始界面
        if(player.pic_index == player.pic_num)
        {
            game_state = GAME_START;
            //begin_flag = 0;
            //关闭定时器
            set_timer(0);
            //暂停一段时间再回到开始界面
            sleep(1);
        }

        int i, j;

        for(i = 0; i <PLAN_MAX_BULLET; i++)
        {
            if(planbullet[i].life == 1)
                show_one_element(&game_engine, planbullet, i);
        }

        for(i = 0; i < MAX_BULLET; i++)
        {
            if(bullet[i].life == 1)
                show_one_element(&game_engine, bullet, i);
        }
        //------从新获取敌人的坐标-------------
        for(i = 0; i < MAX_SMALL_ENEMY; i++)
        {
            get_enemy_pos(&small_enemy[i], 1);
        }
        for(i = 0; i < MAX_MID_ENEMY; i++)
        {
            get_enemy_pos(&mid_enemy[i], 3);
        }

        //-----------显示飞机------------
        show_one_element(&game_engine, &player, 0);
        //-----------显示敌人------------
        show_all_element(&game_engine, mid_enemy, MAX_MID_ENEMY);
        show_all_element(&game_engine, small_enemy, MAX_SMALL_ENEMY);
        //------------------显示分数----------------
        show_score();
        //----------刷新窗体-------------
        SDL_Flip(game_engine.window);
    }
}




void init_game()
{
    init_SDL();
    init_rand();
    init_timer();
    init_player(&player);
    init_begin_background(&begin_background);
    init_background(&background);
    init_bullet(bullet);
    init_small_enemy(small_enemy);
    init_mid_enemy(mid_enemy);
    init_number(number);
}

void init_SDL()
{
    beginSDL(SDL_INIT_VIDEO);
    game_engine.window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BPP, SDL_SWSURFACE);
    if(NULL == game_engine.window)
    {
        perror("SDL_SetVideoMode fail\n");
    }
    //设备窗体的标题及图标
    SDL_WM_SetCaption("坦克大战飞机",NULL);
    game_engine.picture = load_image(game_engine.picture, "./pic/picture.png");
    //开始界面的背景
    game_engine.begin = load_image(game_engine.begin, "./pic/begin_background.png");
    //正常游戏背景
    game_engine.background = load_image(game_engine.background, "./pic/background.png");
    //加载显示数字的图片
    game_engine.score = IMG_Load("./pic/score.png");
    if(NULL == game_engine.score)
    {
        perror("load picture fail\n");
    }
    //除去截取到的图片周围的空白
    SDL_SetColorKey (game_engine.score, SDL_SRCCOLORKEY, SDL_MapRGB(game_engine.score->format, 0, 0, 0));
    //格式化从新输出
    game_engine.score = SDL_DisplayFormat(game_engine.score);
    //===========joystick==================
    SDL_Init(SDL_INIT_EVERYTHING);
    //设置窗口大小和分辨率
    SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BPP, SDL_SWSURFACE);
}

void init_rand()
{ //使用时间作为随机数种子，获取坐标的时候用到
    srand(time(NULL));
}

void init_timer()
{
    //定时器设置
    set_timer(5);
    signal(SIGALRM, timer);
}

void init_begin_background(PLANE *background)
{
    background->src_rect[0].x = 0;
    background->src_rect[0].y = 0;
    background->src_rect[0].w = SCREEN_WIDTH;
    background->src_rect[0].h = SCREEN_HEIGHT;
    background->dst_rect.x = 0;
    background->dst_rect.y = 0;
}

void init_background(PLANE *background)
{
    background->src_rect[0].x = 0;
    background->src_rect[0].y = 560;
    background->src_rect[0].w = SCREEN_WIDTH;
    background->src_rect[0].h = SCREEN_HEIGHT;

    background->dst_rect.x = 0;
    background->dst_rect.y = 0;
}

void init_player(PLANE *player)
{
    player->pic_num = 4;
    player->pic_index = 0;

    player->src_rect[0].x = 431;
    player->src_rect[0].y = 0;
    player->src_rect[0].w = 65;
    player->src_rect[0].h = 80;

    player->src_rect[1].x = 431;
    player->src_rect[1].y = 249;
    player->src_rect[1].w =	65;
    player->src_rect[1].h = 80;

    player->src_rect[2].x = 431;
    player->src_rect[2].y = 83;
    player->src_rect[2].w = 65;
    player->src_rect[2].h = 80;

    player->src_rect[3].x = 431;
    player->src_rect[3].y = 166;
    player->src_rect[3].w = 65;
    player->src_rect[3].h = 80;

    player->dst_rect.x = 130;
    player->dst_rect.y = 480;

    player->speed_x = 0;
    player->speed_y = 0;

    player->life = 1;
}

void init_bullet(PLANE *bullet)
{
    int i;
    for(i = 0; i < MAX_BULLET; i++)
    {
        bullet[i].pic_num = 0;
        bullet[i].pic_index = 0;

        bullet[i].src_rect[0].x = 498;
        bullet[i].src_rect[0].y = 0;
        bullet[i].src_rect[0].w = 8;
        bullet[i].src_rect[0].h = 15;

        bullet[i].dst_rect.x = 0;
        bullet[i].dst_rect.y = 0;

        bullet[i].life = 0;
    }


    for(i = 0; i < PLAN_MAX_BULLET; i++)
    {
        planbullet[i].pic_num = 0;
        planbullet[i].pic_index = 0;

        planbullet[i].src_rect[0].x = 498;
        planbullet[i].src_rect[0].y = 0;
        planbullet[i].src_rect[0].w = 8;
        planbullet[i].src_rect[0].h = 15;

        planbullet[i].dst_rect.x = 0;
        planbullet[i].dst_rect.y = 0;

        planbullet[i].life = 0;
    }

}

void init_small_enemy(PLANE *small_enemy)
{
    int i;
    for(i = 0; i < MAX_SMALL_ENEMY; i++)
    {
        small_enemy[i].pic_num = 4;
        small_enemy[i].pic_index = 0;

        small_enemy[i].src_rect[0].x = 84;
        small_enemy[i].src_rect[0].y = 658;
        small_enemy[i].src_rect[0].w = 32;
        small_enemy[i].src_rect[0].h = 23;

        small_enemy[i].src_rect[1].x = 48;
        small_enemy[i].src_rect[1].y = 657;
        small_enemy[i].src_rect[1].w = 32;
        small_enemy[i].src_rect[1].h = 23;

        small_enemy[i].src_rect[2].x = 420;
        small_enemy[i].src_rect[2].y = 732;
        small_enemy[i].src_rect[2].w = 37;
        small_enemy[i].src_rect[2].h = 28;

        small_enemy[i].src_rect[3].x = 473;
        small_enemy[i].src_rect[3].y = 718;
        small_enemy[i].src_rect[3].w = 40;
        small_enemy[i].src_rect[3].h = 40;

        small_enemy[i].dst_rect.x = 0;
        small_enemy[i].dst_rect.y = 0;
        small_enemy[i].life = 0;
    }
}

void init_mid_enemy(PLANE *mid_enemy)
{
    int i;
    for(i = 0; i < MAX_MID_ENEMY; i++)
    {
        mid_enemy[i].pic_num = 5;
        mid_enemy[i].pic_index = 0;

        mid_enemy[i].src_rect[0].x = 0;
        mid_enemy[i].src_rect[0].y = 569;
        mid_enemy[i].src_rect[0].w = 46;
        mid_enemy[i].src_rect[0].h = 59;

        mid_enemy[i].src_rect[1].x = 432;
        mid_enemy[i].src_rect[1].y = 414;
        mid_enemy[i].src_rect[1].w = 46;
        mid_enemy[i].src_rect[1].h = 62;

        mid_enemy[i].src_rect[2].x = 432;
        mid_enemy[i].src_rect[2].y = 538;
        mid_enemy[i].src_rect[2].w = 46;
        mid_enemy[i].src_rect[2].h = 59;

        mid_enemy[i].src_rect[3].x = 432;
        mid_enemy[i].src_rect[3].y = 600;
        mid_enemy[i].src_rect[3].w = 46;
        mid_enemy[i].src_rect[3].h = 59;

        mid_enemy[i].src_rect[4].x = 432;
        mid_enemy[i].src_rect[4].y = 478;
        mid_enemy[i].src_rect[4].w = 46;
        mid_enemy[i].src_rect[4].h = 58;

        mid_enemy[i].dst_rect.x = 0;
        mid_enemy[i].dst_rect.y = 0;

        mid_enemy[i].life = 0;
    }
}


void init_number(PLANE *number)
{
    number[0].pic_num = 1;
    number[0].pic_index = 0;
    number[0].src_rect[0].x = 32;
    number[0].src_rect[0].y = 0;
    number[0].src_rect[0].w = 16;
    number[0].src_rect[0].h = 32;
    number[0].dst_rect.x = 0;
    number[0].dst_rect.y = 0;
    number[0].life = 1;

    number[1].pic_num = 1;
    number[1].pic_index = 0;
    number[1].src_rect[0].x = 48;
    number[1].src_rect[0].y = 0;
    number[1].src_rect[0].w = 16;
    number[1].src_rect[0].h = 32;
    number[1].dst_rect.x = 0;
    number[1].dst_rect.y = 0;
    number[1].life = 1;

    number[2].pic_num = 1;
    number[2].pic_index = 0;
    number[2].src_rect[0].x = 64;
    number[2].src_rect[0].y = 0;
    number[2].src_rect[0].w = 16;
    number[2].src_rect[0].h = 32;
    number[2].dst_rect.x = 0;
    number[2].dst_rect.y = 0;
    number[2].life = 1;

    number[3].pic_num = 1;
    number[3].pic_index = 0;
    number[3].src_rect[0].x = 80;
    number[3].src_rect[0].y = 0;
    number[3].src_rect[0].w = 16;
    number[3].src_rect[0].h = 32;
    number[3].dst_rect.x = 0;
    number[3].dst_rect.y = 0;
    number[3].life = 1;


    number[4].pic_num = 1;
    number[4].pic_index = 0;
    number[4].src_rect[0].x = 96;
    number[4].src_rect[0].y = 0;
    number[4].src_rect[0].w = 16;
    number[4].src_rect[0].h = 32;
    number[4].dst_rect.x = 0;
    number[4].dst_rect.y = 0;
    number[4].life = 1;

    number[5].pic_num = 1;
    number[5].pic_index = 0;
    number[5].src_rect[0].x = 112;
    number[5].src_rect[0].y = 0;
    number[5].src_rect[0].w = 16;
    number[5].src_rect[0].h = 32;
    number[5].dst_rect.x = 0;
    number[5].dst_rect.y = 0;
    number[5].life = 1;

    number[6].pic_num = 1;
    number[6].pic_index = 0;
    number[6].src_rect[0].x = 128;
    number[6].src_rect[0].y = 0;
    number[6].src_rect[0].w = 16;
    number[6].src_rect[0].h = 32;
    number[6].dst_rect.x = 0;
    number[6].dst_rect.y = 0;
    number[6].life = 1;

    number[7].pic_num = 1;
    number[7].pic_index = 0;
    number[7].src_rect[0].x = 144;
    number[7].src_rect[0].y = 0;
    number[7].src_rect[0].w = 16;
    number[7].src_rect[0].h = 32;
    number[7].dst_rect.x = 0;
    number[7].dst_rect.y = 0;
    number[7].life = 1;

    number[8].pic_num = 1;
    number[8].pic_index = 0;
    number[8].src_rect[0].x = 160;
    number[8].src_rect[0].y = 0;
    number[8].src_rect[0].w = 16;
    number[8].src_rect[0].h = 32;
    number[8].dst_rect.x = 0;
    number[8].dst_rect.y = 0;
    number[8].life = 1;

    number[9].pic_num = 1;
    number[9].pic_index = 0;
    number[9].src_rect[0].x = 176;
    number[9].src_rect[0].y = 0;
    number[9].src_rect[0].w = 16;
    number[9].src_rect[0].h = 32;
    number[9].dst_rect.x = 0;
    number[9].dst_rect.y = 0;
    number[9].life = 1;
}
