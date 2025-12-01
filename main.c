#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>

#define DEVICE_PATH "/dev/input/event4"
#define TARGET_CODE 109     // PageDown 的键码
#define TARGET_VALUE 1      // 按下状态 (1)
#define COMMAND "fbink -s -f &" // 要执行的命令，加上 & 避免阻塞

// 执行自定义命令的函数
void execute_command() {
    // 使用 system 函数执行 Shell 命令
    system(COMMAND);
}

int main(int argc, char *argv[]) {
    int fd;
    // input_event 结构体
    struct input_event ev;
    
    // 1. 打开设备文件，只读
    if ((fd = open(DEVICE_PATH, O_RDONLY)) == -1) {
        perror("Error opening device");
        return EXIT_FAILURE;
    }
    
    printf("Listening for PageDown (%d) on %s...\n", TARGET_CODE, DEVICE_PATH);

    // 2. 循环读取事件流
    while (1) {
        // 每次读取一个 input_event 结构体的大小
        if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            // 检查是否是按键事件 (EV_KEY)
            if (ev.type == EV_KEY && ev.code == TARGET_CODE && ev.value == TARGET_VALUE) {
                
                // 3. 检测到目标按键按下，执行自定义命令
                printf("PageDown detected! Executing command: %s\n", COMMAND);
                execute_command();
            }
        } else {
            perror("Error reading event");
            sleep(1);
        }
    }
    
    close(fd);
    return EXIT_SUCCESS;
}