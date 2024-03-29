/*
 * Arquivo: main.c
 * Autores: Luis Filipe de Lima Sales (GitHub @luisfilipels) e Raimundo Azevedo (GitHub @Neto2047)
 *
 * Descrição: Este algoritmo consiste na execução do algoritmo Flood Fill sobre uma imagem PGM (em formato binário),
 * passando por, antes disso, pelo processamento da imagem pelo algoritmo de Otsu (para determinação do nível ótimo de
 * limiarização), por uma roodada de erosão e outra de dilatação, e por fim, pelo Flood Fill em si, que é utilizado para
 * a contagem de componentes conexas que foram obtidas a partir da imagem binária obtida pelo algoritmo de Otsu. A imagem
 * é, por fim, exportada para outro arquivo, out.pgm, que mostra o resultado dessas operações.
*/

/*
 * HISTÓRICO DE EDIÇÕES:
 * -------------------------------------------------
 * Data: 8/10/19, 7:22 PM
 * Autor: Luis Filipe e Raimundo Azevedo
 * Motivo: Primeiro commit no Git. Base do código, e implementação de Otsu, implmentados dias antes.
 * -------------------------------------------------
 * Data: 9/10/19, 11:44 PM
 * Autor: Luis Filipe
 * Motivo: Flood Fill funcionando.
 * -------------------------------------------------
 * Data: 10/10/19, 8:14 PM
 * Autor: Luis Filipe e Raimundo Azevedo
 * Motivo: Comentários no código.
 * -------------------------------------------------
 * Data: 11/10/19, 2:47 PM
 * Autor: Luis Filipe
 * Motivo: Correções no algoritmo.
 * -------------------------------------------------
 * Data: 16/10/19, 8:16 PM
 * Autor: Raimundo Azevedo
 * Motivo: Lógica equivalente para a função erode.
 * -------------------------------------------------
 * Data: 16/10/19, 10:22 PM
 * Autor: Raimundo Azevedo
 * Motivo: Função Threshold melhorada. Adiçao de comentários em Otsu, Threshold e Flood Fill.
 * -------------------------------------------------
 * Data: 17/10/19, 10:42 PM
 * Autor: Luis Filipe
 * Motivo: Mais comentários. Algoritmo OK!
 * -------------------------------------------------
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

/** @brief A função Threshold executa o algoritmo de Otsu sobre um histograma,
 **        e com isso, determina o valor ótimo de limiarização para a imagem.
 ** @param *hist Ponteiro para array que representa o histograma dos pixels uma imagem
 ** @return Retorna um inteiro representando o valor ótimo de limiarização para uma imagem.
 **/
int Threshold(int *hist){
    int total = 160*120;//quantidade de pixels da imagem
    double gsum = 0;	//soma ponderada global das ocorrencias do pixel por sua intensidade
    double gavg;	//media global ponderada dos pixels
    double n1=0;	//numero de pixels da classe C1
    double n2=0;	//numero de pixels da classe C2
    double m1=0;	//media ponderada dos pixels da classe C1
    double m2=0;	//media ponderada dos pixels da classe C2
    double var;		//variancia entre as classes C1 e C2
    double maxVar=0;	//armazena a maior variância
    int threshold;	//valor para o qual as classes C1 e C2 possuem variância máxima

    for(int i=0;i<256;i++){
        gsum += (double)hist[i]*i;
    }
    gavg = gsum/total;
    for(int i=0;i<256;++i){

	n1 += hist[i];
	//n1-Número de pixels cujas intensidades variam de 0 a i (Classe C1)

	m1 += (double)i*hist[i];
	//m1-Soma usada para a média ponderada das intensidades dos pixels de C1

	n2 = total - n1;
	//n2-Número de pixels cujas intensidades variam de i+1 a 255 (Classe C2)

	m2 = gsum - m1;
	//m2-Soma usada para a média ponderada das intensidades dos pixels de C2

	var = (n1/total)*((m1/n1)-gavg)*((m1/(n1))-gavg)+
		(n2/total)*((m2/n2)-gavg)*((m2/n2)-gavg);
	//var-Variância entre classes para essa aplicação conforme descrito em:
	//GONZALEZ, Rafael C. WOODS, Richard E. EDDINS, Steven L. Digital Image
	//Processing using MATLAB. 2a edicao. Gatesmark Publishing. 2009.

	if(var > maxVar){
	    maxVar = var;
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
  *                 1 0 0 0 0 0 0               1 1 0 1 1 0 0
  *                 0 0 0 1 1 0 0     ---->     1 0 1 1 1 1 0
  *                 0 0 0 1 1 0 0               0 0 1 1 1 1 0
  *                 0 0 0 0 0 0 0               0 0 0 1 1 0 0
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

/** @brief floodFill determina a area conectada a um dado pixel da imagem obtida.
  * Se o pixel vizinho é de foreground (255) este é preenchido com targetColor e 
  * inserido na fila sendo posteriormente marcado como visitado e removido da
  * fila ao verificar seus vizinhos. A função encerra quando a fila for vazia.
  *
  * @param binaryMatrix[120][160] matriz de pixels da imagem binária
  * @param x indica a posicão do pixel relativa às linhas da matriz
  * @param y indica a posição do pixel relativas às colunas da matriz
  * @param visited[120][160] visited[x][y] indica se o pixel (x,y) foi visitado (1) ou não (0)
  * @param targetColor inteiro entre 0 e 255 indicando uma cor em gray scale para preenchimento
  */
void floodFill (int binaryMatrix[120][160], int x, int y, int visited[120][160], int targetColor) {
    Queue *queue = createQueue((unsigned int) ((120 * 160) / 2));
    push(queue, x, y);
    int currentX = 0;
    int currentY = 0;
    while (!isEmpty(queue)) {
        dequeue(queue, &currentX, &currentY);//remove da fila e atualiza a posicao atual
        binaryMatrix[currentX][currentY] = targetColor;//preenche o pixel atual com targetColor
        visited[currentX][currentY] = 1;//marca o pixel como visitado
	//verifica vizinho acima, se ele for pixel de foreground e não
	//foi visitado é preenchido e inserido na fila
        if (isValid(binaryMatrix, currentX+1, currentY, visited, 255)){
            binaryMatrix[currentX+1][currentY] = targetColor;
            push(queue, currentX+1, currentY);
        }
	//verifica vizinho abaixo, análogo ao caso anterior
        if (isValid(binaryMatrix, currentX-1, currentY, visited, 255)){
            binaryMatrix[currentX-1][currentY] = targetColor;
            push(queue, currentX-1, currentY);
	}
	//verifica vizinho a direita, análogo ao caso anterior
        if (isValid(binaryMatrix, currentX, currentY+1, visited, 255)){
            binaryMatrix[currentX][currentY+1] = targetColor;
            push(queue, currentX, currentY+1);
        }
	//verifica vizinho a esquerda, análogo ao caso anterior
        if (isValid(binaryMatrix, currentX, currentY-1, visited, 255)){
            binaryMatrix[currentX][currentY-1] = targetColor;
            push(queue, currentX, currentY-1);
        }
    }
    free(queue->arrayX);
    free(queue->arrayY);
    free(queue);
}



/**
 * @brief A função runAlgorithm executa todos os algoritmos já desenvolvidos. Primeiro é lido o caminho para um arquivo
 * pgm que se deseja que se execute as funções. Essa imagem é lida e armazenada na memória. Enquanto ela está sendo lida,
 * o histograma da imagem também está sendo gerado, histograma este que é posteriormente passado para a função Threshold.
 * Feito isso, e com o valor ótimo de limiarização obtido, gera-se uma imagem binária que posteriormente passa por erosão
 * e dilatação. Por fim, o Flood Fill é executado duas vezes, na primeira vez para se contar a quantidade de componentes
 * conexas, e na segunda para pintar essas componentes de forma a haver uma distribuição uniforme de cores entre todas
 * as componentes.
 * @return
 */
int runAlgorithm() {
    FILE *file2read;	// Ponteiro do arquivo para leitura
    FILE *file2write;	// Ponteiro do arquivo para escrita
    char path[256]="";  // Buffer usado para armazenar o caminho para o arquivo
    printf("Informe o nome do arquivo, ou seu caminho e nome: ");
    fflush(stdout);
    scanf("%s",path);
    file2read = fopen(path,"r");
    file2write = fopen("out.pgm","w+");

    if((file2read == NULL) || (file2write == NULL)){
        printf("Falha ao abrir arquivos\n");
        exit(1);
    }

    int hist[256];	       // Histograma de pixels
    int matrix [120][160]; // Matriz usada para armazenar a imagem lida em memória. Operações são feitas nessa matriz.
    int visited[120][160]; // Matriz que indica quais pixeis já foram visitados, em diferentes ocasiões.

    for(int i=0;i<256;++i) hist[i] = 0; // Iniciamos o histograma apenas com 0s.

    for (int j = 0; j < 15; j++) {
        fgetc(file2read);
        // Descartamos os primeiros 15 caracteres do arquivo, pois não importam para este algoritmo.
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            visited[h][w] = 0;              // Todos os visitados iniciados em 0.
            int c = (unsigned char)fgetc(file2read);
            matrix[h][w] = c;               // Armazenamos na memória o valor de um pixel.
            hist[matrix[h][w]]++;           // Incrementamos a cor do pixel atual no histograma.
        }
    }
    
    int t = Threshold(hist);
    printf("t = %d", t);

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matrix[h][w] < t) {           // Se o valor de um pixel for menor que o limiar, pintá-lo de preto.
                matrix[h][w] = 0;
            } else {                          // Se não, pintá-lo de branco.
                matrix[h][w] = 255;
            }
        }
    }

    erode(matrix, visited);         // Realizamos uma erosão para limpar pixels "soltos" na imagem.
    dilate(matrix, visited);        // Em seguida, uma dilatação, para preservar o tamanho dos elementos.


    int connectedComps = 0;

    int originalMatrix [120][160];
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            originalMatrix[h][w] = matrix[h][w];
            // Salvamos a matriz original, para que depois de contar o número de componentes conexas, possamos pintá-la
            // na cor correta.
        }
    }

    int originalVisited[120][160];
    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            originalVisited[h][w] = visited[h][w];
            // Salvamos também a matriz de visitados, pelo mesmo motivo anterior.
        }
    }

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (matrix[h][w] == 255 && !visited[h][w]) {    // Se o pixel atual for branco e não tiver sido visitado
                connectedComps++;                           // aumentamos a contagem de componentes
                floodFill(matrix, h, w, visited, 80);       // e preenchemos aquela componente com uma cor qualquer.
            }
        }
    }

    // Esta é a cor da primeira componente conexa.
    int targetColor = 40;
    if (connectedComps == 0) {
        connectedComps = 1;
    }
    int rate = (255 - 40) / connectedComps;
    // Este é o incremento de cor que é executado na pintura de uma componente para outra

    for (int h = 0; h < 120; h++) {
        for (int w = 0; w < 160; w++) {
            if (originalMatrix[h][w] == 255 && !originalVisited[h][w]) {
                if (targetColor >= 255) targetColor = 40;                      // Se a cor a ser pintada extrapolar o limite, resetar.
                targetColor += rate;                                           // Cor da próxima componente conexa.
                floodFill(originalMatrix, h, w, originalVisited, targetColor); // Pintamos a componente atual com targetColor.
            }
        }
    }

    printf("\nconnectedComps = %d", connectedComps);
    printf("\ntargetColor = %d", targetColor);

    // Inicamos a escrita no arquivo de output do algoritmo.
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
    runAlgorithm();
}
