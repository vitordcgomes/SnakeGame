#include <stdio.h>
#include <stdlib.h>

#define TAM_FILE 1000
#define TAM_MAPA 100
#define TAM_COBRA 100
#define PONTO_DINHEIRO 10
#define PONTO_COMIDA 1
#define CRESCIMENTO 1

#define PAREDE '#'
#define VAZIO ' '
#define TUNEL '@'
#define COMIDA '*'
#define DINHEIRO '$'
#define CORPO 'o'
#define MORTE 'X'
#define HEAD_DIREITA '>'
#define HEAD_ESQUERDA '<'
#define HEAD_CIMA '^'
#define HEAD_BAIXO 'v'

#define MOV_HORARIO 'h'
#define MOV_ANT_HORARIO 'a'
#define MOV_CONTINUAR 'c'


typedef struct {
    int linha;
    int coluna;
    int qtdComida;
    int qtdDinheiro;
    char matriz[TAM_MAPA][TAM_MAPA];
} tMapa;

typedef struct {
    int linha;
    int coluna;
    int matriz[TAM_MAPA][TAM_MAPA];
} tHeatMap;

typedef struct {
    char tamanho[TAM_COBRA];
    int posicaoCorpo[TAM_COBRA][2]; //0==linha e 1==coluna
    int tamAtual;
    int posicaoLinha;
    int posicaoColuna;
    int pontos;
    int qtdComida;
    int qtdDinheiro;
    int flagComidaAcabou;
    int flagColisao;
} tCobra;

typedef struct {
    char movimento;
    int numMov;
    int numMovSemPontuar;
    int numMovBaixo;
    int numMovCima;
    int numMovEsquerda;
    int numMovDireita;
} tJogadas;

typedef struct {
    tMapa mapa;
    tHeatMap heatMap;
    tCobra cobra;
    tJogadas jogadas;
} tJogo;


//________________________________IMPRESSAO______________________________________
void ImprimeMatriz(tMapa map);
void ImprimeMatrizNoArquivo(tMapa map, FILE *file);
void ImprimeHeatMapNoArquivo(tHeatMap heatMap, FILE *file);
void ImprimeEstadoDoJogo(tJogadas jogadas, tCobra cobra, tMapa mapa); 
//_______________________________________________________________________________


//_________________________________ARQUIVOS______________________________________
void CriaArquivoEstatisticas(tJogadas jogadas, char *argv[]); 
void CriaArquivoHeatMap(tHeatMap heatMap, char *argv[]); 
void CriaArquivoRanking(tHeatMap heatMap, char *argv[]);
void AdicionaArquivoSaida(tJogadas jogadas, tCobra cobra, tMapa mapa, char *argv[]); 
void AdicionaArquivoResumo(tMapa mapa, tJogadas jogadas, tCobra cobra, char *argv[]); 
//_______________________________________________________________________________


//________________________________INICIALIZACOES_________________________________
tMapa InicializaJogo(int argc, char *argv[]);
tHeatMap InicializaHeatMap(int linha, int coluna);
tCobra InicializaCobra(tMapa mapa);
tJogadas InicializaContagemMovimentos();
//_______________________________________________________________________________


//_____________________________________JOGO______________________________________
int IdentificaFinalDaCobra(char cobra[TAM_COBRA]); 
int VerificaComida(tMapa mapa);
void OficialJogo (tMapa mapa, tHeatMap tHeatMap, tCobra cobra, tJogadas jogadas, int argc, char *argv[]);
tJogo ModificaParametrosJogo(tJogo jogo, char *argv[]);
tJogo PassaDaBorda(tJogo jogo);
tJogo MataCobra(tJogo jogo);
tCobra CresceCobra(tCobra cobra);
tJogo TratarTuneis(tJogo jogo);
//________________________________________________________________________________


//**************************************MAIN************************************
int main(int argc, char *argv[]) {
    
    tMapa mapa;
    tHeatMap heatMap;
    tCobra cobra;
    tJogadas jogadas;

    //inicializacao das variaveis que serao usadas para o funcionamento do jogo
    mapa=InicializaJogo(argc, argv);
    heatMap=InicializaHeatMap(mapa.linha, mapa.coluna);
    cobra=InicializaCobra(mapa);
    jogadas=InicializaContagemMovimentos();

    OficialJogo(mapa, heatMap, cobra, jogadas, argc, argv);

    return 0;
}
//*******************************************************************************


//________________________________IMPRESSAO______________________________________
void ImprimeMatriz(tMapa map) {
    int i, j;

    for (i=0;i<map.linha;i++) {
        for (j=0;j<map.coluna;j++) {
            printf("%c", map.matriz[i][j]);
        }
        printf("\n");
    }
}

void ImprimeMatrizNoArquivo(tMapa map, FILE *file) {
     int i, j;

    for (i=0;i<map.linha;i++) {
        for (j=0;j<map.coluna;j++) {
            fprintf(file, "%c", map.matriz[i][j]);
        }
        fprintf(file, "\n");
    }
}

void ImprimeHeatMapNoArquivo(tHeatMap heatMap, FILE *file) {
    int i, j;

    for (i=0;i<heatMap.linha;i++) {
        for (j=0;j<heatMap.coluna;j++) {

            if (j==heatMap.coluna-1) {
                fprintf(file, "%d", heatMap.matriz[i][j]);
            }

            else fprintf(file, "%d ", heatMap.matriz[i][j]);
        }
        fprintf(file, "\n");
    }
}

void ImprimeEstadoDoJogo(tJogadas jogadas, tCobra cobra, tMapa mapa) {
    printf("\n");
    printf ("Estado do jogo apos o movimento '%c':\n", jogadas.movimento);
    
    ImprimeMatriz(mapa);

    printf ("Pontuacao: %d\n", cobra.pontos);

    if (cobra.qtdComida==mapa.qtdComida) {
        printf("Voce venceu!\nPontuacao final: %d\n", cobra.pontos);
    }

    if (cobra.tamanho[0]==MORTE) {
        printf("Game over!\nPontuacao final: %d\n", cobra.pontos);
    }
} 
//_______________________________________________________________________________


//_________________________________ARQUIVOS______________________________________
void CriaArquivoEstatisticas(tJogadas jogadas, char *argv[]) {
    char fileEstatisticas[TAM_FILE];
    FILE *estatisticas;

    sprintf(fileEstatisticas, "%s/saida/estatisticas.txt", argv[1]);
    estatisticas = fopen (fileEstatisticas, "w");

    fprintf(estatisticas, "Numero de movimentos: %d\n", jogadas.numMov);
    fprintf(estatisticas, "Numero de movimentos sem pontuar: %d\n", jogadas.numMovSemPontuar);
    fprintf(estatisticas, "Numero de movimentos para baixo: %d\n", jogadas.numMovBaixo);
    fprintf(estatisticas, "Numero de movimentos para cima: %d\n", jogadas.numMovCima);
    fprintf(estatisticas, "Numero de movimentos para esquerda: %d\n", jogadas.numMovEsquerda);
    fprintf(estatisticas, "Numero de movimentos para direita: %d\n", jogadas.numMovDireita);

    fclose(estatisticas);
} 

void CriaArquivoHeatMap(tHeatMap heatMap, char *argv[]) {
    char fileHeatMap[TAM_FILE];
    FILE *heat_Map;

    sprintf(fileHeatMap, "%s/saida/heatmap.txt", argv[1]);
    heat_Map = fopen(fileHeatMap, "w");

    ImprimeHeatMapNoArquivo(heatMap, heat_Map);

    fclose(heat_Map);
}

void CriaArquivoRanking(tHeatMap heatMap, char *argv[]) {
    int i, j, k, maior=0;

    for (i=0;i<heatMap.linha;i++) {
        for (j=0;j<heatMap.coluna;j++) {
            if (heatMap.matriz[i][j]>=maior) {
                maior=heatMap.matriz[i][j];
            }
        }
    }

    char fileRanking[TAM_FILE];
    FILE *ranking;

    sprintf(fileRanking, "%s/saida/ranking.txt", argv[1]);
    ranking = fopen(fileRanking, "w");

    for (k=maior;k>0;k--) {
        for (i=0;i<heatMap.linha;i++) {
            for (j=0;j<heatMap.coluna;j++) {
                if (heatMap.matriz[i][j]==k) {
                    fprintf(ranking, "(%d, %d) - %d\n", i, j, k);
                }
            }
        }
    }

    fclose(ranking);
}

void AdicionaArquivoSaida(tJogadas jogadas, tCobra cobra, tMapa mapa, char *argv[]) {
    char fileSaida[TAM_FILE];
    FILE *saida;

    sprintf(fileSaida, "%s/saida/saida.txt", argv[1]);
    saida = fopen (fileSaida, "a");

    fprintf(saida, "\n");
    fprintf(saida, "Estado do jogo apos o movimento '%c':\n", jogadas.movimento);

    ImprimeMatrizNoArquivo(mapa, saida);

    fprintf(saida, "Pontuacao: %d\n", cobra.pontos);

    if (cobra.qtdComida==mapa.qtdComida) {
        fprintf(saida, "Voce venceu!\nPontuacao final: %d\n", cobra.pontos);
    }

    if (cobra.tamanho[0]==MORTE) {
        fprintf(saida, "Game over!\nPontuacao final: %d\n", cobra.pontos);
    }

    fclose(saida);
}

void AdicionaArquivoResumo(tMapa mapa, tJogadas jogadas, tCobra cobra, char *argv[]) {
    char fileResumo[TAM_FILE];
    FILE *resumo;

    sprintf(fileResumo, "%s/saida/resumo.txt", argv[1]);
    resumo = fopen (fileResumo, "a+");

     if (cobra.qtdComida==mapa.qtdComida) {
        fprintf(resumo, "Movimento %d (%c) fez a cobra crescer para o tamanho %d, terminando o jogo\n", 
                        jogadas.numMov, jogadas.movimento, cobra.tamAtual);
    }

    if (mapa.matriz[cobra.posicaoLinha][cobra.posicaoColuna]==PAREDE || mapa.matriz[cobra.posicaoLinha][cobra.posicaoColuna]==CORPO) {
        fprintf(resumo, "Movimento %d (%c) resultou no fim de jogo por conta de colisao\n", jogadas.numMov, jogadas.movimento);
    }

    if (mapa.matriz[cobra.posicaoLinha][cobra.posicaoColuna]==DINHEIRO) {
        fprintf(resumo, "Movimento %d (%c) gerou dinheiro\n", jogadas.numMov, jogadas.movimento);
    }

    if (mapa.matriz[cobra.posicaoLinha][cobra.posicaoColuna]==COMIDA && cobra.qtdComida!=mapa.qtdComida) {
        fprintf(resumo, "Movimento %d (%c) fez a cobra crescer para o tamanho %d\n", jogadas.numMov, jogadas.movimento, cobra.tamAtual);
    }

    fclose(resumo);
} 
//_______________________________________________________________________________


//________________________________INICIALIZACOES_________________________________
tMapa InicializaJogo(int argc, char *argv[]) {

    char fileNameMap[TAM_FILE], fileNameInt[TAM_FILE];
    int i, j;
    tMapa map;

    FILE *mapa; //'mapa'==nome do ponteiro do tipo FILE, enquanto 'map'== variavel do tipo estruturado tMapa. 'mapa' != 'map'.

    sprintf (fileNameMap, "%s/mapa.txt", argv[1]);
    mapa = fopen(fileNameMap, "r"); 

    if (argc<=1) {
        printf("ERRO: O diretorio de arquivos de configuracao nao foi informado\n");
        exit(0);
    }

    if (mapa == NULL) {
        printf("Nao foi possivel abrir o arquivo 'mapa.txt' pelo caminho '%s'\n", fileNameMap);
        exit(0);
    }

    fscanf(mapa, "%d %d%*c", &map.linha, &map.coluna);

    //leitura da matriz do jogo contida no arquivo mapa.txt:
    for (i=0;i<map.linha;i++) {
        for (j=0;j<map.coluna;j++) {
            fscanf(mapa, "%c", &map.matriz[i][j]);
        }
        if (i!=map.linha-1) {
            fscanf(mapa, "%*c");
        }
        
    }
    fclose(mapa);

    FILE *inicializacao;

    sprintf (fileNameInt, "%s/saida/inicializacao.txt", argv[1]);

    inicializacao = fopen(fileNameInt, "w"); 

    int posInicialLinha, posInicialColuna;
    //Impressao da matriz e posicao inicial da cobra no arquivo inicializacao.txt:

    for (i=0;i<map.linha;i++) {
        for (j=0;j<map.coluna;j++) {
            fprintf(inicializacao, "%c", map.matriz[i][j]);

            if (map.matriz[i][j] == HEAD_DIREITA) {
                posInicialLinha=i+1;
                posInicialColuna=j+1;
            }
        }
        fprintf(inicializacao, "\n");
    }
    fprintf(inicializacao, "A cobra comecara o jogo na linha %d e coluna %d\n", posInicialLinha, posInicialColuna);

    fclose(inicializacao);

    map.qtdComida=0;
    map.qtdDinheiro=0;
    //verificacao da quantidade de comida e dinheiro no mapa:

    for (i=0;i<map.linha;i++) {
        for (j=0;j<map.coluna;j++) {
            if (map.matriz[i][j]==COMIDA) {
                map.qtdComida++;
            }
            else if (map.matriz[i][j]==DINHEIRO) {
                map.qtdDinheiro++;
            }
        }
    }

    return map;
}

tHeatMap InicializaHeatMap(int linha, int coluna) {
    int i, j;
    tHeatMap heatMap;

    heatMap.linha=linha;
    heatMap.coluna=coluna;

    for (i=0;i<linha;i++) {
        for (j=0;j<coluna;j++) {
            heatMap.matriz[i][j]=0;
        }
    }

    return heatMap;
}

tCobra InicializaCobra(tMapa mapa) {
    tCobra cobra;
    int i, j;

    cobra.flagColisao=0;
    cobra.flagComidaAcabou=0;
    cobra.pontos=0;
    cobra.qtdComida=0;
    cobra.qtdDinheiro=0;
    cobra.tamAtual=1;
    cobra.tamanho[0]=HEAD_DIREITA;
    for (i=1;i<TAM_COBRA;i++) {
        cobra.tamanho[i]='\0';
    }

    for (i=0;i<mapa.linha;i++) {
        for (j=0;j<mapa.coluna;j++) {
            if (mapa.matriz[i][j]==HEAD_DIREITA) {
                cobra.posicaoLinha=i;
                cobra.posicaoColuna=j;
            }
        }
    }

    cobra.posicaoCorpo[0][0]=cobra.posicaoLinha;
    cobra.posicaoCorpo[0][1]=cobra.posicaoColuna;
    
    return cobra;
}

tJogadas InicializaContagemMovimentos() {
    tJogadas jogadas;

    jogadas.numMov=0;
    jogadas.numMovBaixo=0;
    jogadas.numMovCima=0;
    jogadas.numMovDireita=0;
    jogadas.numMovEsquerda=0;
    jogadas.numMovSemPontuar=0;

    return jogadas;
}
//_______________________________________________________________________________


//_____________________________________JOGO______________________________________
int IdentificaFinalDaCobra(char cobra[TAM_COBRA]) {
    int i, num;

    for (i=1;i<TAM_COBRA;i++) {
        if (cobra[i]=='\0') {
            num=i;
            return num;
        }
    }
}

int VerificaComida(tMapa mapa) {
    int i, j;

    for (i=0;i<mapa.linha;i++) {
        for (j=0;j<mapa.coluna;j++) {
            if (mapa.matriz[i][j]==COMIDA) {
                return 1;
            }
        }
    }
    return 0;
}

void OficialJogo(tMapa mapa, tHeatMap heatMap, tCobra cobra, tJogadas jogadas, int argc, char *argv[]) {

    int i, j, auxLinha, auxColuna, aux2Linha, aux2Coluna;

    tJogo jogo;
    jogo.mapa=mapa;
    jogo.heatMap=heatMap;
    jogo.cobra=cobra;
    jogo.jogadas=jogadas;

    printf("Bem-vindo ao jogo da cobrinha! :)\n");
    printf("Para jogar, digite 'h', 'a' ou 'c' para movimento horario, anti-horario e continuar reto, respectivamente.\n");
    printf("Bom jogo!\n\n");
    ImprimeMatriz(mapa);
    //printf("\n%d %d\n", mapa.qtdComida, mapa.qtdDinheiro); //printf de teste para verificar qtd de comida e dinheiro no mapa

    jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;

    jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=VAZIO;

    while (scanf("%c%*c", &jogo.jogadas.movimento)==1) {
        //printf("\njogada: %c\n", jogo.jogadas.movimento); //printf de teste para verificar leitura do movimento
        

        if (jogo.jogadas.movimento==MOV_HORARIO) { 
            jogo.jogadas.numMov++;

            if (jogo.cobra.tamanho[0]==HEAD_DIREITA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_BAIXO;
                jogo.cobra.posicaoLinha++;
                jogo.cobra.posicaoCorpo[0][0]++;
                jogo.jogadas.numMovBaixo++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //_________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }
            
            else if (jogo.cobra.tamanho[0]==HEAD_ESQUERDA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_CIMA;
                jogo.cobra.posicaoLinha--;
                jogo.cobra.posicaoCorpo[0][0]--;
                jogo.jogadas.numMovCima++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_CIMA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_DIREITA;
                jogo.cobra.posicaoColuna++;
                jogo.cobra.posicaoCorpo[0][1]++;
                jogo.jogadas.numMovDireita++;

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_BAIXO) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_ESQUERDA;
                jogo.cobra.posicaoColuna--;
                jogo.cobra.posicaoCorpo[0][1]--;
                jogo.jogadas.numMovEsquerda++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

               
               if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

           if (jogo.cobra.flagColisao==1 || jogo.cobra.flagComidaAcabou==1) {
                //GameOver
                break;
            }
        }            

        else if (jogo.jogadas.movimento==MOV_ANT_HORARIO) {
            jogo.jogadas.numMov++;

            if (jogo.cobra.tamanho[0]==HEAD_DIREITA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_CIMA;
                jogo.cobra.posicaoLinha--;
                jogo.cobra.posicaoCorpo[0][0]--;
                jogo.jogadas.numMovCima++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_ESQUERDA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_BAIXO;
                jogo.cobra.posicaoLinha++;
                jogo.cobra.posicaoCorpo[0][0]++;
                jogo.jogadas.numMovBaixo++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_CIMA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_ESQUERDA;
                jogo.cobra.posicaoColuna--;
                jogo.cobra.posicaoCorpo[0][1]--;
                jogo.jogadas.numMovEsquerda++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_BAIXO) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.tamanho[0]=HEAD_DIREITA;
                jogo.cobra.posicaoColuna++;
                jogo.cobra.posicaoCorpo[0][1]++;
                jogo.jogadas.numMovDireita++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            if (jogo.cobra.flagColisao==1 || jogo.cobra.flagComidaAcabou==1) {
                //GameOver
                break;
            }
            
        }     
            
        else if (jogo.jogadas.movimento==MOV_CONTINUAR) {
            jogo.jogadas.numMov++;

            if (jogo.cobra.tamanho[0]==HEAD_DIREITA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.posicaoColuna++;
                jogo.cobra.posicaoCorpo[0][1]++;
                jogo.jogadas.numMovDireita++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_ESQUERDA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.posicaoColuna--;
                jogo.cobra.posicaoCorpo[0][1]--;
                jogo.jogadas.numMovEsquerda++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //_______________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_CIMA) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.posicaoLinha--;
                jogo.cobra.posicaoCorpo[0][0]--;
                jogo.jogadas.numMovCima++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //_________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            else if (jogo.cobra.tamanho[0]==HEAD_BAIXO) {

                auxLinha = jogo.cobra.posicaoLinha;
                auxColuna = jogo.cobra.posicaoColuna;

                jogo.cobra.posicaoLinha++;
                jogo.cobra.posicaoCorpo[0][0]++;
                jogo.jogadas.numMovBaixo++;

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
                    jogo = TratarTuneis(jogo);
                }

                jogo = PassaDaBorda(jogo);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {
                    jogo.cobra.tamAtual+=CRESCIMENTO;
                }

                //______________________________Movimenta corpo da cobra_________________________________
                for(i=1;i<jogo.cobra.tamAtual;i++) {

                    aux2Linha = jogo.cobra.posicaoCorpo[i][0];
                    aux2Coluna = jogo.cobra.posicaoCorpo[i][1];

                    jogo.cobra.posicaoCorpo[i][0]=auxLinha;
                    jogo.cobra.posicaoCorpo[i][1]=auxColuna;

                    auxLinha = aux2Linha;
                    auxColuna= aux2Coluna;
                }

                for (i=1;i<jogo.cobra.tamAtual;i++) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=CORPO;
                }
                //________________________________________________________________________________________

                jogo = ModificaParametrosJogo(jogo, argv);

                if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]!=TUNEL) {
                    jogo.mapa.matriz[jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][0]][jogo.cobra.posicaoCorpo[jogo.cobra.tamAtual-1][1]]=VAZIO;
                }
            }

            if (jogo.cobra.flagColisao==1 || jogo.cobra.flagComidaAcabou==1) {
                //GameOver
                break;
            }
        }

    }   

    CriaArquivoEstatisticas(jogo.jogadas, argv);
    CriaArquivoHeatMap(jogo.heatMap, argv);
    CriaArquivoRanking(jogo.heatMap, argv);

}

tJogo ModificaParametrosJogo(tJogo jogo, char *argv[]) {
    
    if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==TUNEL) {
        jogo = TratarTuneis(jogo);
    }

    jogo = PassaDaBorda(jogo);
    
                       
    if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==COMIDA) {

        jogo.cobra.qtdComida+=1;
        jogo.cobra.pontos+=PONTO_COMIDA;
        jogo.cobra=CresceCobra(jogo.cobra);

        if (jogo.cobra.qtdComida == jogo.mapa.qtdComida) {
            jogo.cobra.flagComidaAcabou=1;
        }

        AdicionaArquivoResumo(jogo.mapa, jogo.jogadas, jogo.cobra, argv);
        jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=jogo.cobra.tamanho[0];
        jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;

        AdicionaArquivoSaida(jogo.jogadas, jogo.cobra, jogo.mapa, argv);
        ImprimeEstadoDoJogo(jogo.jogadas, jogo.cobra, jogo.mapa);

    }

    else if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==DINHEIRO) {

        jogo.cobra.qtdDinheiro+=1;
        jogo.cobra.pontos+=PONTO_DINHEIRO;

        AdicionaArquivoResumo(jogo.mapa, jogo.jogadas, jogo.cobra, argv);
        jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=jogo.cobra.tamanho[0];
        jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;
    
        AdicionaArquivoSaida(jogo.jogadas, jogo.cobra, jogo.mapa, argv);
        ImprimeEstadoDoJogo(jogo.jogadas, jogo.cobra, jogo.mapa);
    }

    else if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==CORPO) {

        jogo.jogadas.numMovSemPontuar++;
        jogo.cobra.flagColisao=1;
        
        AdicionaArquivoResumo(jogo.mapa, jogo.jogadas, jogo.cobra, argv);
        jogo=MataCobra(jogo);
        jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=jogo.cobra.tamanho[0];
        jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;

        AdicionaArquivoSaida(jogo.jogadas, jogo.cobra, jogo.mapa, argv);
        ImprimeEstadoDoJogo(jogo.jogadas, jogo.cobra, jogo.mapa);
    }

    else if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==PAREDE) {

        jogo.jogadas.numMovSemPontuar++;
        jogo.cobra.flagColisao=1;
        jogo=MataCobra(jogo);

        AdicionaArquivoResumo(jogo.mapa, jogo.jogadas, jogo.cobra, argv);
        jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=jogo.cobra.tamanho[0];
        jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;

        AdicionaArquivoSaida(jogo.jogadas, jogo.cobra, jogo.mapa, argv);
        ImprimeEstadoDoJogo(jogo.jogadas, jogo.cobra, jogo.mapa);
    }


    else if (jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]==VAZIO) {

        jogo.jogadas.numMovSemPontuar++;
        jogo.mapa.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]=jogo.cobra.tamanho[0];
        jogo.heatMap.matriz[jogo.cobra.posicaoLinha][jogo.cobra.posicaoColuna]+=1;

        AdicionaArquivoSaida(jogo.jogadas, jogo.cobra, jogo.mapa, argv);
        ImprimeEstadoDoJogo(jogo.jogadas, jogo.cobra, jogo.mapa);
    }


    return jogo;
}

tJogo PassaDaBorda(tJogo jogo) {
    if (jogo.cobra.posicaoLinha>=jogo.mapa.linha) {
        jogo.cobra.posicaoLinha=0;
        jogo.cobra.posicaoCorpo[0][0]=0;
    }
    else if (jogo.cobra.posicaoLinha<0) {
        jogo.cobra.posicaoLinha=jogo.mapa.linha-1;
        jogo.cobra.posicaoCorpo[0][0]=jogo.mapa.linha-1;
    }


    if (jogo.cobra.posicaoColuna>=jogo.mapa.coluna) {
        jogo.cobra.posicaoColuna=0;
        jogo.cobra.posicaoCorpo[0][1]=0;
    }
    else if (jogo.cobra.posicaoColuna<0) {
        jogo.cobra.posicaoColuna=jogo.mapa.coluna-1;
        jogo.cobra.posicaoCorpo[0][1]=jogo.mapa.coluna-1;
    }

    return jogo;
}

tJogo MataCobra(tJogo jogo) {
    int i=0;

    for (i=0;i<=jogo.cobra.tamAtual;i++) {
        jogo.cobra.tamanho[i]=MORTE;
    }

    for (i=1;i<jogo.cobra.tamAtual;i++) {
        jogo.mapa.matriz[jogo.cobra.posicaoCorpo[i][0]][jogo.cobra.posicaoCorpo[i][1]]=MORTE;
    }

    return jogo;
}

tCobra CresceCobra(tCobra cobra) {
    int i;

    for(i=1;i<cobra.tamAtual;i++) {
        cobra.tamanho[i]=CORPO;
    }

    return cobra;
}

tJogo TratarTuneis(tJogo jogo) {
    int i, j, x, y;


    for (i=0;i<jogo.mapa.linha;i++) {
        for (j=0;j<jogo.mapa.coluna;j++) {

            if (jogo.mapa.matriz[i][j]==TUNEL) {

                x=i;
                y=j;

                if (x!=jogo.cobra.posicaoLinha || y!=jogo.cobra.posicaoColuna) {
                    if (jogo.cobra.tamanho[0]==HEAD_DIREITA) {
                        jogo.cobra.posicaoLinha=x;
                        jogo.cobra.posicaoColuna=y+1;

                        jogo.cobra.posicaoCorpo[0][0]=x;
                        jogo.cobra.posicaoCorpo[0][1]=y+1;
                    }
                    else if (jogo.cobra.tamanho[0]==HEAD_ESQUERDA) {
                        jogo.cobra.posicaoLinha=x;
                        jogo.cobra.posicaoColuna=y-1;

                        jogo.cobra.posicaoCorpo[0][0]=x;
                        jogo.cobra.posicaoCorpo[0][1]=y-1;
                    }
                    else if (jogo.cobra.tamanho[0]==HEAD_CIMA) {
                        jogo.cobra.posicaoLinha=x-1;
                        jogo.cobra.posicaoColuna=y;

                        jogo.cobra.posicaoCorpo[0][0]=x-1;
                        jogo.cobra.posicaoCorpo[0][1]=y;
                    }
                    else if (jogo.cobra.tamanho[0]==HEAD_BAIXO) {
                        jogo.cobra.posicaoLinha=x+1;
                        jogo.cobra.posicaoColuna=y;

                        jogo.cobra.posicaoCorpo[0][0]=x+1;
                        jogo.cobra.posicaoCorpo[0][1]=y;
                    }

                    return jogo;
                }
            }
        }
    }
}
//_______________________________________FIM!_____________________________________
//Vitor Dadalto Camara Gomes - Tabalho Eng Comp Ufes Prog 1 - 1o periodo - 10/08/2022