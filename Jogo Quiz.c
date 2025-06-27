#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define MAX_PERGUNTAS 10
#define TAM_PERGUNTA 256
#define TAM_RESPOSTA 100
#define NUM_FASES 7
#define MAX_JOGADORES 100

typedef struct {
    char pergunta[TAM_PERGUNTA];
    char respostas[4][TAM_RESPOSTA];
    int resposta_correta;
    int dificuldade;
} Pergunta;

typedef struct {
    Pergunta **perguntas;
    int num_perguntas;
    char nome[50];
    int pontos_necessarios;
} Fase;

typedef struct {
    char nome[50];
    int pontuacao;
    int fase_alcancada;
} Jogador;

Fase fases[NUM_FASES];
Jogador placares[MAX_JOGADORES];
int num_jogadores = 0;
int fase_atual = 0;
char nome_jogador[50];

void carregar_fase(Fase *fase, const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    int i, j;

    if (arquivo == NULL) {
        printf("Erro ao carregar fase: %s\n", nome_arquivo);
        exit(1);
    }

    fscanf(arquivo, "%49[^\n]\n", fase->nome);
    fscanf(arquivo, "%d\n", &fase->num_perguntas);
    fase->pontos_necessarios = (int)(fase->num_perguntas * 0.6);

    fase->perguntas = (Pergunta**)malloc(fase->num_perguntas * sizeof(Pergunta*));

    for (i = 0; i < fase->num_perguntas; i++) {
        fase->perguntas[i] = (Pergunta*)malloc(sizeof(Pergunta));

        fscanf(arquivo, " %d\n", &fase->perguntas[i]->dificuldade);
        fgets(fase->perguntas[i]->pergunta, TAM_PERGUNTA, arquivo);
        fase->perguntas[i]->pergunta[strcspn(fase->perguntas[i]->pergunta, "\n")] = '\0';

        for (j = 0; j < 4; j++) {
            fgets(fase->perguntas[i]->respostas[j], TAM_RESPOSTA, arquivo);
            fase->perguntas[i]->respostas[j][strcspn(fase->perguntas[i]->respostas[j], "\n")] = '\0';
        }

        fscanf(arquivo, "%d\n", &fase->perguntas[i]->resposta_correta);
    }

    fclose(arquivo);
}

void liberar_fase(Fase *fase) {
    int i;
    for (i = 0; i < fase->num_perguntas; i++) {
        free(fase->perguntas[i]);
    }
    free(fase->perguntas);
}

int jogar_fase(Fase fase) {
    int acertos = 0;
    int perguntas_restantes = fase.num_perguntas;
    bool *respondidas = (bool*)calloc(fase.num_perguntas, sizeof(bool));
    int i, j, indice, resposta;

    while (perguntas_restantes > 0) {
        system("cls");
        printf("=== %s ===\n", fase.nome);
        printf("Perguntas restantes: %d\n", perguntas_restantes);
        printf("-----------------------------\n");

        indice = -1;
        for (i = 0; i < fase.num_perguntas; i++) {
            if (!respondidas[i]) {
                indice = i;
                break;
            }
        }

        if (indice == -1) break;

        Pergunta *p = fase.perguntas[indice];
        printf("Pergunta %d:\n%s\n", p->dificuldade, p->pergunta);

        for (j = 0; j < 4; j++) {
            printf("%d) %s\n", j+1, p->respostas[j]);
        }

        printf("\nSua resposta (1-4): ");
        scanf("%d", &resposta);
        while (getchar() != '\n');

        if (resposta == p->resposta_correta) {
            printf("\nCorreto!\n");
            acertos++;
        } else {
            printf("\nErrado!\n");
        }

        respondidas[indice] = true;
        perguntas_restantes--;
        printf("Pontuacao atual: %d/%d\n", acertos, fase.num_perguntas);
        printf("\nPressione Enter para continuar...");
        while (getchar() != '\n');
    }

    free(respondidas);
    system("cls");
    printf("=== RESULTADO: %s ===\n", fase.nome);
    printf("Acertos: %d/%d\n", acertos, fase.num_perguntas);

    if (acertos >= fase.pontos_necessarios) {
        printf("\nParabens! Voce passou para a proxima fase!\n");
    } else {
        printf("\nVoce nao atingiu os 60%% necessarios. Tente novamente!\n");
    }

    printf("\nPressione Enter para continuar...");
    while (getchar() != '\n');
    return acertos;
}

void tela_estatisticas() {
    system("cls");
    printf("=============================\n");
    printf("       ESTATISTICAS         \n");
    printf("=============================\n");

    if (num_jogadores == 0) {
        printf("\nNenhum placar registrado ainda.\n");
    } else {
        printf("\nTop Jogadores:\n");
        printf("--------------------------------------------------\n");
        printf("Nome               Pontuacao   Fase Alcancada\n");
        printf("--------------------------------------------------\n");

        int i;
        for (i = 0; i < num_jogadores; i++) {
            printf("%-18s %-11d %d\n",
                   placares[i].nome,
                   placares[i].pontuacao,
                   placares[i].fase_alcancada);
        }
    }

    printf("\nPressione Enter para continuar...");
    while (getchar() != '\n');
}

void salvar_placar() {
    FILE *arquivo = fopen("placar.dat", "wb");
    if (arquivo == NULL) {
        printf("Erro ao salvar placar.\n");
        return;
    }

    fwrite(&num_jogadores, sizeof(int), 1, arquivo);
    fwrite(placares, sizeof(Jogador), num_jogadores, arquivo);
    fclose(arquivo);
}

void carregar_placar() {
    FILE *arquivo = fopen("placar.dat", "rb");
    if (arquivo == NULL) {
        num_jogadores = 0;
        return;
    }

    fread(&num_jogadores, sizeof(int), 1, arquivo);
    fread(placares, sizeof(Jogador), num_jogadores, arquivo);
    fclose(arquivo);
}

void adicionar_jogador(int pontuacao, int fase_alcancada) {
    if (num_jogadores < MAX_JOGADORES) {
        strcpy(placares[num_jogadores].nome, nome_jogador);
        placares[num_jogadores].pontuacao = pontuacao;
        placares[num_jogadores].fase_alcancada = fase_alcancada;
        num_jogadores++;
        salvar_placar();
    }
}

void tela_principal() {
    int opcao;
    bool jogo_completo;
    int i, acertos;

    do {
        system("cls");
        printf("=============================\n");
        printf("Bem-vindo, %s!\n", nome_jogador);
        printf("=============================\n");
        printf("1. Iniciar Jogo\n");
        printf("2. Estatisticas\n");
        printf("3. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        while (getchar() != '\n');

        switch(opcao) {
            case 1:
                fase_atual = 0;
                jogo_completo = false;
                while (fase_atual < NUM_FASES && !jogo_completo) {
                    acertos = jogar_fase(fases[fase_atual]);

                    if (acertos >= fases[fase_atual].pontos_necessarios) {
                        fase_atual++;
                    } else {
                        printf("\nRepetindo a fase %d...\n", fase_atual+1);
                        printf("\nPressione Enter para continuar...");
                        while (getchar() != '\n');
                    }

                    if (fase_atual == NUM_FASES) {
                        printf("\nParabens! Voce completou todas as fases!\n");
                        adicionar_jogador(acertos, NUM_FASES);
                        printf("\nPressione Enter para continuar...");
                        while (getchar() != '\n');
                        jogo_completo = true;
                    }
                }
                break;
            case 2:
                tela_estatisticas();
                break;
            case 3:
                printf("Saindo do jogo...\n");
                break;
            default:
                printf("Op��o invalida!\n");
                printf("\nPressione Enter para continuar...");
                while (getchar() != '\n');
        }
    } while (opcao != 3);
}

void tela_inicial() {
    system("cls");
    printf("=============================\n");
    printf("       JOGO DE QUIZ         \n");
    printf("=============================\n");
    printf("\nDigite seu nome: ");
    fgets(nome_jogador, 50, stdin);
    nome_jogador[strcspn(nome_jogador, "\n")] = '\0';

    tela_principal();
}

int main() {
    int i;
    char nome_arquivo[20];

    carregar_placar();

    for (i = 0; i < NUM_FASES; i++) {
        sprintf(nome_arquivo, "fase_%d.txt", i+1);
        carregar_fase(&fases[i], nome_arquivo);
    }

    tela_inicial();

    for (i = 0; i < NUM_FASES; i++) {
        liberar_fase(&fases[i]);
    }

    return 0;
}

