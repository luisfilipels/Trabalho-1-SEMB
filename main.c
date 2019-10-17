/*
 *Arquivo: main.c
 *Autores: Luis Filipe de Lima Sales (GitHub @luisfilipels) e Raimundo Azevedo 
 *
 *Descrição: Este algoritmo consiste na execução do algoritmo Flood Fill sobre uma imagem PGM (em formato binário),
 *passando por, antes disso, pelo processamento da imagem pelo algoritmo de Otsu (para determinação do nível ótimo de
 *limiarização), por uma roodada de erosão e outra de dilatação, e por fim, pelo Flood Fill em si, que é utilizado para
 *a contagem de componentes conexas que foram obtidas a partir da imagem binária obtida pelo algoritmo de Otsu. A imagem
 *é, por fim, exportada para outro arquivo, out.pgm, que mostra o resultado dessas operações.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/*----------------------------------------------INIT QUEUE----------------------------------------------*/

/*
 * Devido à restrição no uso de memória com a execução deste algoritmo, utilizamos uma fila baseada em array (ao invés 
 * de lista ligada), pois assim alocamos a matriz unidimensional da fila apenas uma vez, em sua inicialização na função
 * Flood Fill.
*/

typedef struct Queue {
    /*
     * Utilizamos inteiros que representam a frente da fila, a sua traseira, e seu tamanho, para que não sejam perdidos
     * durante a manipulação da fila.
    */
    int front, rear, size;
    unsigned capacity; // Capacidade máxima da fila
    int *arrayX; // Ponteiro para a fila que armazena uma coordenada no eixo X da imagem.
    int *arrayY; // Idem, para o eixo Y.
} Queue;

/** @brief A função createQueue inicializa uma fila e a retorna como ponteiro. É executado apenas no Flood Fill, uma vez.
  * @param capacity O tamanho do array a ser criado.
  * @return Retorna um ponteiro para uma nova fila, inicializada.
  */
Queue *createQueue (unsigned capacity) {
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue)); // Criamos a struct.
    queue->capacity = capacity;         // Atribuimos sua capacidade.
    queue->front = queue->size = 0;     // Dizemos que sua frente é a posição 0 do array.
    queue->rear = capacity - 1;         // Sua traseira é a última posição do array.
    queue->arrayX = (int*) malloc(queue->capacity * sizeof(int)); // Criamos uma fila para coordenadas X
    queue->arrayY = (int*) malloc(queue->capacity * sizeof(int)); // e outra para coordenadas Y.
    return queue;
}

/** @brief A função isFull determina se a fila passada por parâmetro está cheia ou não.
  * @param *queue Ponteiro para a fila
  * @return Retorna 1 se a fila está cheia, 0 caso contrário.
  */
int isFull (Queue *queue) {
    return (queue->size == queue->capacity);
}


/** @brief A função isEmpty determina se a fila passada por parâmetro está vazia ou não.
  * @param *queue Ponteiro para a fila
  * @return Retorna 1 se a fila está vazia, 0 caso contrário.
  */
int isEmpty (Queue *queue) {
    return (queue->size == 0);
}

/** @brief A função push insere dois elementos, um na fila do eixo X, outro na fila do eixo Y.
  * @param *queue Ponteiro para a fila que se deseja fazer push.
  * @param X Inteiro a ser inserido em X.
  * @param Y Inteiro a ser inserido em Y.
  */
void push (Queue *queue, int X, int Y) {
    if (isFull(queue)) return;  // Não fazer nada se estiver cheia.
    queue->rear = (queue->rear+1) % queue->capacity; // Array circular. A traseira é incrementada, módulo capacidade.
    queue->arrayX[queue->rear] = X; // Inserimos X em arrayX.
    queue->arrayY[queue->rear] = Y; // Inserimos Y em arrayY.
    queue->size = queue->size + 1; // Incrementamos o tamanho da fila.
}

/** @brief dequeue Retira da fila elementos na frente da fila. Retorna esses elementos por referência.
  * @param *x ponteiro para o elemento do eixo X a ser retornado
  * @param *y ponteiro para o elemento do eixo Y a ser retornado
  */
void dequeue (Queue *queue, int *x, int *y) {
    if (isEmpty(queue)) { // Não fazer nada se estiver vazia.
        return;
    }
    *x = queue->arrayX[queue->front]; // x recebe o valor que estava na frente da fila
    *y = queue->arrayY[queue->front]; // Idem para y.
    queue->front = (queue->front + 1)%queue->capacity; // Incrementamos a fila em 1.
    queue->size = queue->size - 1; // Diminuimos em 1 o tamanho da fila.
}

/*----------------------------------------------END QUEUE----------------------------------------------*/

/*-----------------------------------------INIT OTSU THRESHOLD-----------------------------------------*/

/** @brief A função Threshold executa o algoritmo de Otsu sobre um histograma, e com isso, determina o valor ótimo de limiarização para a imagem.
  * @param *hist Ponteiro para array que representa o histograma de uma imagem
  * @return Retorna um inteiro representando o valor ótimo de limiarização para uma imagem.
  */
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

/*-----------------------------------------END OTSU THRESHOLD-----------------------------------------*/

/*------------------------------------------INIT FLOOD FILL-------------------------------------------*/

/** @brief A função isValid serve tanto para determinar se valores x e y estão dentro dos limites de
  * uma imagem 160x120, como para verificar se a posição indicada por esses dois números não foi já
  * utilizada, e para se certificar que o valor dessa posição é o mesmo valor que um certo comp.
  * @param binaryMatrix[120][160] Matriz binária representando a imagem passada a limiarização.
  * @param x Posição x na matriz
  * @param y Posição y na matriz
  * @param visited[120][160] Matriz que indica se cada uma de suas posições já foi visitada no Flood Fill 
  * @param comp Valor a ser comparado.
  */
int isValid (int binaryMatrix[120][160], int x, int y, int visited[120][160], int comp) {
    // Se x e y forem posições válidas, com um valor correto e que não tenham sido visitadas, retorne 1.
    if (x >= 0 && x < 120 && y >= 0 && y < 160 && binaryMatrix[x][y] == comp && visited[x][y] != 1) {
        return 1;
    }
    return 0;
}

/** @brief A função erode aplica uma transformação de erosão na imagem, que consiste em tirar um pixel
  * do exterior de cada "objeto". Por exemplo:
  *                 0 0 0 0 0 0 0               0 0 0 0 0 0 0 
  *                 1 0 0 1 1 0 0               0 0 0 0 0 0 0     
  *                 0 0 1 1 1 1 0     ---->     0 0 0 1 1 0 0
  *                 0 0 1 1 1 1 0               0 0 0 1 1 0 0
  *                 0 0 0 1 1 0 0               0 0 0 0 0 0 0
  *                 0 0 0 0 0 0 0               0 0 0 0 0 0 0
  * Com a execução dessa função, conseguimos eliminar pixels individuais que fiquem "soltos" na imagem,
  * para que não sejam considerados uma componente conexa. É executada antes do dilate, para limpar a imagem.
  * @param outBinaryMatrix[120][160] Matriz resultante da operação, passada por referência.
  * @param visited[120][160] Matriz de visitados
  */
void erode (int outBinaryMatrix[120][160], int visited[120][160]) {
    int orBinaryMatrix[120][160]; // Criamos uma matriz para guardar os valores originais da matriz binária.
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            orBinaryMatrix[h][w] = outBinaryMatrix[h][w]; 
            /* Armazenamos os valores originais, para comparação posterior.
             * Isso é nececessário para que façamos a erosão sobre os pixels da imagem original, não da imagem
             * alterada pela erosão.
             */
        }
    }
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (orBinaryMatrix[h][w] == 255) {  // Se o pixel estiver "pintado"
                if ( (isValid(orBinaryMatrix, h, w-1, visited, 0) ||
                      isValid(orBinaryMatrix, h, w+1, visited, 0) || 
                      isValid(orBinaryMatrix, h+1, w, visited, 0) ||
                      isValid(orBinaryMatrix, h-1, w, visited, 0)     
                      /* As verificações anteriores servem para determinar se algum dos vizinhos de um pixel branco
                       * (vizinhos apenas em cima e em baixo, esquerda e direita), é preto. Se for, o pixel atual
                       * fica preto, pois isso significa que estamos na borda de um objeto.
                       */
                )) {
                    outBinaryMatrix[h][w] = 0;
                }
            }
        }
    }
}

/** @brief A função dilate é análoga à erode, fazendo, porém, o contrário. Ou seja, aplica uma transformação de 
  * dilatação na imagem, que consiste em pintar de branco cada pixel do exterior imediato de cada "objeto". Exemplo:
  *                 0 0 0 0 0 0 0               1 0 0 0 0 0 0 
  *                 1 0 0 1 1 0 0               1 1 0 1 1 0 0     
  *                 0 0 1 1 1 1 0     ---->     1 0 1 1 1 1 0
  *                 0 0 1 1 1 1 0               0 0 1 1 1 1 0
  *                 0 0 0 1 1 0 0               0 0 0 1 1 0 0
  *                 0 0 0 0 0 0 0               0 0 0 0 0 0 0
  * Com a execução dessa função, conseguimos restaurar ao tamanho original cada objeto da imagem, após a execução do dilate.
  * @param outBinaryMatrix[120][160] Matriz resultante da operação, passada por referência.
  * @param visited[120][160] Matriz de visitados
  */
void dilate (int outBinaryMatrix[120][160], int visited[120][160]) {
    int orBinaryMatrix[120][160]; // Criamos uma matriz para guardar os valores originais da matriz binária.
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            orBinaryMatrix[h][w] = outBinaryMatrix[h][w];
            /* Armazenamos os valores originais, para comparação posterior.
             * Isso é nececessário para que façamos a erosão sobre os pixels da imagem original, não da imagem
             * alterada pela erosão.
             */
        }
    }
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (orBinaryMatrix[h][w] == 0) { // Se o pixel atual for preto.
                if (isValid(orBinaryMatrix, h, w-1, visited, 255) ||
                    isValid(orBinaryMatrix, h, w+1, visited, 255) ||
                    isValid(orBinaryMatrix, h+1, w, visited, 255) ||
                    isValid(orBinaryMatrix, h-1, w, visited, 255)
                    /* As verificações anteriores servem para determinar se algum vizinho de um pixel preto é
                     * branco. Se for, pintamos o pixel atual de branco.
                     */
                ) {
                    outBinaryMatrix[h][w] = 255;
                }
            }
        }
    }
}

/** @brief floodFill
  *
  */
void floodFill (int binaryMatrix[120][160], int x, int y, int visited[120][160], int targetColor) {
    Queue *queue = createQueue((unsigned int) ((120 * 160) / 2));
    push(queue, x, y);
    int currentX = 0;
    int currentY = 0;
    while (!isEmpty(queue)) {
        dequeue(queue, &currentX, &currentY);
        binaryMatrix[currentX][currentY] = targetColor;
        visited[currentX][currentY] = 1;
        if (isValid(binaryMatrix, currentX+1, currentY, visited, 255)){
            binaryMatrix[currentX+1][currentY] = targetColor;
            push(queue, currentX+1, currentY);
        }
        if (isValid(binaryMatrix, currentX-1, currentY, visited, 255)){
            binaryMatrix[currentX-1][currentY] = targetColor;
            push(queue, currentX-1, currentY);
        }
        if (isValid(binaryMatrix, currentX, currentY+1, visited, 255)){
            binaryMatrix[currentX][currentY+1] = targetColor;
            push(queue, currentX, currentY+1);
        }
        if (isValid(binaryMatrix, currentX, currentY-1, visited, 255)){
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
        //strcpy(num, itoa(b, num, 10));
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
    FILE *file2read, *file2write;
    int i=0;
    int hist[256];
    char buffer[256]="";
    printf("Informe o nome do arquivo: ");
    scanf("%s",buffer);
    file2read = fopen(buffer,"r");
    file2write = fopen("out.pgm","w+");

    if((file2read == NULL) || (file2write == NULL)){
        printf("Falha ao abrir arquivos\n");
        exit(1);
    }

    int matrix [120][160];
    int visited[120][160];

    for(i=0;i<256;++i) hist[i] = 0;

    for (int j = 0; j < 15; j++) {
        fgetc(file2read);
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            visited[h][w] = 0;
            int c = fgetc(file2read);
            matrix[h][w] = c;
            hist[matrix[h][w]]++;
        }
    }

    int t = Threshold(hist);
    printf("t = %d", t);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matrix[h][w] < t) {
                matrix[h][w] = 0;
            } else {
                matrix[h][w] = 255;
            }
        }
    }

    erode(matrix, visited);
    dilate(matrix, visited);


    int connectedComps = 0;

    int originalMatrix [120][160];
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            originalMatrix[h][w] = matrix[h][w];
        }
    }

    int originalVisited[120][160];
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            originalVisited[h][w] = visited[h][w];
        }
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matrix[h][w] == 255 && !visited[h][w]) {
                connectedComps++;
                floodFill(matrix, h, w, visited, 80);
            }
        }
    }

    int targetColor = 40;
    int rate = (255 - 40) / connectedComps;

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (originalMatrix[h][w] == 255 && !originalVisited[h][w]) {
                if (targetColor >= 255) targetColor = 40;
                targetColor += rate;
                floodFill(originalMatrix, h, w, originalVisited, targetColor);
            }
        }
    }

    printf("\nconnectedComps = %d", connectedComps);
    printf("\ntargetColor = %d", targetColor);

    fputs("P5\n", file2write);
    fputs("160 120\n", file2write);
    fputs("255\n", file2write);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            fputc(originalMatrix[h][w], file2write);
        }
    }

    fclose(file2read);
    fclose(file2write);
    return 0;
}

int main() {
    //testThreshold();
    runThreshold();
    int matrix[160][120];
}
