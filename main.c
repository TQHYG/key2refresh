#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <sys/wait.h> // 需要包含 waitpid
#include <signal.h>   // 需要包含信号处理

#define DEVICE_PATH "/dev/input/event4"
#define TARGET_CODE 109     // PageDown 的键码
#define TARGET_VALUE 1      // 按下状态 (1)
// 命令不变，它会在后台产生一个子进程
#define COMMAND "fbink -s -f &" 

// --- SIGCHLD 处理函数：回收僵尸子进程 ---
void sigchld_handler(int sig) {
    // 必须使用循环来回收所有僵尸子进程，并使用 WNOHANG 避免阻塞
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // 子进程被回收，这里可以添加日志输出，但为了精简不加
    }
}

// --- 信号处理器设置 ---
void setup_signal_handlers() {
    struct sigaction sa_chld;
    struct sigaction sa_term;
    
    // 1. 设置 SIGCHLD (子进程终止信号) 处理
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; 
    
    if (sigaction(SIGCHLD, &sa_chld, 0) == -1) {
        perror("sigaction SIGCHLD failed");
        // 在嵌入式设备上，如果失败则可能导致僵尸进程，但我们允许继续运行
    }
    
    // 2. 设置 SIGTERM (终止信号) 的默认处理，确保程序能优雅退出
    // 或者你可以定义一个 cleanup_and_exit 函数来执行安全退出逻辑
    sa_term.sa_handler = SIG_DFL;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, 0); 
}

// 执行自定义命令的函数
void execute_command() {
    // system 会阻塞直到命令执行完成。
    // 因为 COMMAND 结尾有 '&'，system 会在后台启动 Shell 并立即返回。
    // 子进程（fbink）会由这个 Shell 启动。
    system(COMMAND);
}

int main(int argc, char *argv[]) {
    int fd;
    struct input_event ev;
    
    // 在主循环开始前，设置信号处理器
    setup_signal_handlers(); 
    
    if ((fd = open(DEVICE_PATH, O_RDONLY)) == -1) {
        perror("Error opening device");
        return EXIT_FAILURE;
    }
    
    printf("Listening for PageDown (%d) on %s...\n", TARGET_CODE, DEVICE_PATH);

    while (1) {
        if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.code == TARGET_CODE && ev.value == TARGET_VALUE) {
                
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