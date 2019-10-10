#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/*INIT QUEUE*/

typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    int *arrayX;
    int *arrayY;
} Queue;

Queue *createQueue (unsigned capacity) {
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->arrayX = (int*) malloc(queue->capacity * sizeof(int));
    queue->arrayY = (int*) malloc(queue->capacity * sizeof(int));
    return queue;
}

int isFull (Queue *queue) {
    return (queue->size == queue->capacity);
}

int isEmpty (Queue *queue) {
    return (queue->size == 0);
}

void push (Queue *queue, int X, int Y) {
    if (isFull(queue)) return;
    queue->rear = (queue->rear+1) % queue->capacity; // Array circular
    queue->arrayX[queue->rear] = X;
    queue->arrayY[queue->rear] = Y;
    queue->size = queue->size + 1;
}

void dequeue (Queue *queue, int *x, int *y) {
    if (isEmpty(queue)) {
        return;
    }
    *x = queue->arrayX[queue->front];
    *y = queue->arrayY[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
}

int frontX(Queue* queue)
{
    if (isEmpty(queue)) {
        return INT_MIN;
    }
    return queue->arrayX[queue->front];
}

int frontY(Queue* queue)
{
    if (isEmpty(queue)) {
        return INT_MIN;
    }
    return queue->arrayY[queue->front];
}

int rearX(Queue* queue)
{
    if (isEmpty(queue)) {
        return INT_MIN;
    }
    return queue->arrayX[queue->rear];
}

int rearY(Queue* queue)
{
    if (isEmpty(queue)) {
        return INT_MIN;
    }
    return queue->arrayY[queue->rear];
}

/*END QUEUE*/

/*INIT OTSU THRESHOLD*/

int Threshold(int *hist){
    int total = 160 * 120;
    double soma = 0;
    for (int i = 0; i < 256; i++) {
        soma += i * hist[i];
    }

    double soma2 = 0;
    int pesoBack = 0;
    int pesoFront = 0;

    double varMax = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) {
        pesoBack += hist[i];
        if (pesoBack == 0) continue;

        pesoFront = total - pesoBack;
        if (pesoFront == 0) break;

        soma2 += (double) i * hist[i];

        double mediaBack = soma2 / pesoBack;
        double mediaFront = (soma - soma2) / pesoFront;

        double varEntre = (double) pesoBack * (double) pesoFront * (mediaBack - mediaFront) * (mediaBack - mediaFront);

        if (varEntre > varMax) {
            varMax = varEntre;
            threshold = i;
        }
    }
    return threshold;

}

int isValid (int binaryMatrix[120][160], int x, int y, int visited[120][160]) {
    if (x >= 0 && x < 120 && y >= 0 && y < 160 && binaryMatrix[x][y] == 255 && visited[x][y] != 1) {
        return 1;
    }
    return 0;
}

void floodFill (int binaryMatrix[120][160], int x, int y, int visited[120][160], int targetColor) {
    Queue *queue = createQueue((unsigned int) ((120 * 160) / 2));
    push(queue, x, y);
    int currentX = 0;
    int currentY = 0;
    while (!isEmpty(queue)) {
        dequeue(queue, &currentX, &currentY);
        binaryMatrix[currentX][currentY] = targetColor;
        visited[currentX][currentY] = 1;
        if (isValid(binaryMatrix, currentX+1, currentY, visited)){
            binaryMatrix[currentX+1][currentY] = targetColor;
            push(queue, currentX+1, currentY);
        }
        if (isValid(binaryMatrix, currentX-1, currentY, visited)){
            binaryMatrix[currentX-1][currentY] = targetColor;
            push(queue, currentX-1, currentY);
        }
        if (isValid(binaryMatrix, currentX, currentY+1, visited)){
            binaryMatrix[currentX][currentY+1] = targetColor;
            push(queue, currentX, currentY+1);
        }
        if (isValid(binaryMatrix, currentX, currentY-1, visited)){
            binaryMatrix[currentX][currentY-1] = targetColor;
            push(queue, currentX, currentY-1);
        }
    }
}

int testThreshold () {
    FILE *file2read;
    int i=0;
    int hist[256];
    char buffer[256]="";
    char buffer2[256];
    printf("Informe o nome do arquivo: ");
    scanf("%s",buffer);
    file2read = fopen(buffer,"r");


    if(file2read == NULL){
        printf("Falha ao abrir arquivos\n");
        exit(1);
    }

    int matriz [120][160];
    int visitados[120][160];

    for(i=0;i<256;++i) hist[i] = 0;

    for (int j = 0; j < 15; j++) {
        fgetc(file2read);
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            visitados[h][w] = 0;
            int c = fgetc(file2read);
            matriz[h][w] = c;
            hist[matriz[h][w]]++;
        }
    }

    int t = Threshold(hist);
    printf("t = %d", t);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matriz[h][w] < t) {
                matriz[h][w] = 0;
            } else {
                matriz[h][w] = 255;
            }
        }
    }

    int matrizbkp[120][160];
    for (int k = 0; k < 120; k++) {
        for (int j = 0; j < 160; j++) {
            matrizbkp[k][j] = matriz[k][j];
        }
    }

    FILE *file2write;

    for (int b = 0; b < 76; b++) {
        char path[256];
        char num[10];
        int count = 0;
        strcpy(path, "out");
        strcpy(num, itoa(b, num, 10));
        strcat(path, num);
        strcat(path, ".pgm");
        file2write = fopen(path, "w+");
        int flag = 0;
        for (int h = 0; h < 120; h++) {
            for (int w = 0; w < 160; w++) {
                if (matriz[h][w] == 255 && !visitados[h][w]) {
                    count++;
                    if (h == 27 && w == 121) {
                        printf("Here");
                    }
                    floodFill(matriz, h, w, visitados, 60);
                    if (count >= b) {
                        flag = 1;
                        break;
                    }
                }
            }
            if (flag) break;
        }

        fputs("P5\n", file2write);
        fputs("160 120\n", file2write);
        fputs("255\n", file2write);

        for (int h = 0; h < 120; h++) {
            for (int w = 0; w < 160; w++) {
                fputc(matriz[h][w], file2write);
            }
        }
        for (int m = 0; m < 120; m++) {
            for (int j = 0; j < 160; j++) {
                matriz[m][j] = matrizbkp[m][j];
                visitados[m][j] = 0;
            }
        }
        fclose(file2write);
    }

    fclose(file2read);

    return 0;
}

int runThreshold () {
    FILE *file2read, *file2write, *aux;
    int i=0,pos=0;
    int npxl=0,mg=0;
    int hist[256];
    char *strnum = malloc(3*sizeof(char));
    char buffer[256]="";
    char num[3];
    printf("Informe o nome do arquivo: ");
    scanf("%s",buffer);
    file2read = fopen(buffer,"r");
    file2write = fopen("out.pgm","w+");

    if((file2read == NULL) || (file2write == NULL)){
        printf("Falha ao abrir arquivos\n");
        exit(1);
    }

    int matriz [120][160];
    int visitados[120][160];

    for(i=0;i<256;++i) hist[i] = 0;

    for (int j = 0; j < 15; j++) {
        fgetc(file2read);
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            visitados[h][w] = 0;
            int c = fgetc(file2read);
            matriz[h][w] = c;
            hist[matriz[h][w]]++;
        }
    }

    int t = Threshold(hist);
    printf("t = %d", t);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matriz[h][w] < t) {
                matriz[h][w] = 0;
            } else {
                matriz[h][w] = 255;
            }
        }
    }

    int count = 0;

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matriz[h][w] == 255 && !visitados[h][w]) {
                count++;
                if (count == 255) count = 0;
                floodFill(matriz, h, w, visitados, count);
            }
        }
    }

    printf("\ncount = %d", count);

    fputs("P5\n", file2write);
    fputs("160 120\n", file2write);
    fputs("255\n", file2write);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            fputc(matriz[h][w], file2write);
        }
    }

    fclose(file2read);
    fclose(file2write);
    return 0;
}

int main() {
    testThreshold();
    //runThreshold();
    int matrix[160][120];
}