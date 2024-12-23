#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/mount.h>
#include <stdint-gcc.h>
#include <sys/stat.h>
 
#define BUFFER_SIZE 1024

//sighup
void handle_SIGHUP(int signal) {
    if (signal == SIGHUP) {
        printf("Configuration reloaded\n");
        exit(0);
    }
}

//10
void disk_check(char* dname) {
 
    while (*dname == ' ') {
        dname++;
    }
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "/dev/%s", dname);//в массив fp размером 128 записываем диск, формируем полный путь

    FILE* disk = fopen(full_path, "rb");//открываем устройство для чтения
    if (disk == NULL) {
        printf("error\n");
        return;
    }
 
    if (fseek(disk, 510, SEEK_SET) != 0) {//переходим к 510 байту
        printf("error\n");
        fclose(disk);
        return;
    }
    uint8_t signature[2];
    if (fread(signature, 1, 2, disk) != 2) {//считываем последние 2 байта (1 объект размером 2)
        printf("error\n");
        fclose(disk);
        return;
    }
    fclose(disk);
    if (signature[0] == 0x55 && signature[1] == 0xAA) {
        printf("disk %s является загрузочным.\n", dname);
    } else {
        printf("disk %s не является загрузочным.\n", dname);
    }
}

//12
bool append(char* path1, char* path2) {
    FILE *f1 = fopen(path1, "a");
    FILE *f2 = fopen(path2, "r");
    if (!f1 || !f2) {
        printf("Error while reading file %s\n", path2);
        return false;
    }
    char buf[256];

    while (fgets(buf, 256, f2) != NULL) {
        fputs(buf, f1);
    }
    fclose(f1);
    fclose(f2);
    return true;
}

void makeDump(DIR* dir, char* path) {
    FILE* res = fopen("res.txt", "w+");
    fclose(res);
    struct dirent* ent;//структура для возврата информации о входах в католог
    char* file_path;
    while ((ent = readdir(dir)) != NULL) {//чтение следующего входа из католога, если входов нет - ошибка
        asprintf(&file_path, "%s/%s", path, ent->d_name); //запись в выделенную строку пути и пути к файлу внутри каталога
        if(!append("res.txt", file_path)) {
            return;
        }
    }
    printf("succesfull dump\n");
}

 
int main() {
    char input[BUFFER_SIZE];
    
// По сигналу SIGHUP вывести "Configuration reloaded"
        signal (SIGHUP, handle_SIGHUP);
        FILE* file_history = fopen("history.txt", "a");
    do {
        printf("USER$ ");
        fflush(stdout);
        bool check=false;

        // input
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            printf("\nЗавершение работы (Ctrl+D)\n");
            break; // выход из цикла при EOF
        }
 
        // \n
        input[strcspn(input, "\n")] = '\0';
 
        // exit
        if (strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) {
            printf("Завершение работы (exit/\\q)\n");
            break;
        }

        // history
        if(file_history) {
            fprintf(file_history, "%s\n", input);
          fclose(file_history);
          file_history = fopen("history.txt", "a");
        }

        //echo $PATH
        if(strcmp(input, "e $PATH")==0) {
            check=true;
            char *path = getenv("PATH");
            if (path!=NULL){
                printf("%s\n", path);
            }
            else {
                 printf("error \n");
            }
            continue;
        }
 
        // echo
        if (strncmp(input, "echo ", 5) == 0) {
            check=true;
            printf("%s\n", input + 5);
            continue;
        }

       //binary

        if (strncmp(input, "run ", 4) == 0){
            check=true;
            pid_t p = fork();//pid_t тип данных для id процесса, fork создает новый процесс
            if (p == 0){
              char *argv[] = { "sh", "-c", input + 4, 0 };//sh двоичный исп файл, который мы запускаем
              execvp(argv[0], argv);
              fprintf(stderr, "Failed to exec shell on %s", input + 4);
              
              exit(1);
             
            }
sleep(1);
continue;
}


  
        
       //10. По `\l /dev/sda` определить является ли диск загрузочным
         if (strncmp(input, "\\l", 2) == 0) {
            check=true;
            char* dname = input + 3;
            disk_check(dname);
            continue;
        }
        
       //12
        if (strncmp(input, "\\mem ", 5) == 0) {
            char* path;
            asprintf(&path, "/proc/%s/map_files", input+5);//вывод данных в выделенную в памяти строку

            DIR* dir = opendir(path);//открытие потока католога
            if (dir) {
                makeDump(dir, path);
            }
            else {
                printf("Process not found\n");
            }
            check = true;
            continue;
        }

        if(check==false){
        printf("there is no command: %s\n", input);
        }
       
    }
    while (!feof(stdin));
 
    
 
    return 0;
}
