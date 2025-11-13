#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ITENS 10
#define TAM_NOME  32
#define TAM_TIPO  20

typedef struct {
    char nome[TAM_NOME];
    char tipo[TAM_TIPO];
    int  quantidade;
    int  prioridade; // 1..5
} Item;

/* ================== Utils ================== */
static void tira_enter(char *s){
    if(!s) return;
    size_t n = strlen(s);
    if(n && s[n-1] == '\n') s[n-1] = '\0';
}
static void ler_linha(char *buf, int cap){
    if(fgets(buf, cap, stdin)) tira_enter(buf);
    else if(cap>0) buf[0] = '\0';
}
static int stricmp_pt(const char *a, const char *b){
    for(;;){
        unsigned char ca = (unsigned char)tolower((unsigned char)*a);
        unsigned char cb = (unsigned char)tolower((unsigned char)*b);
        if(ca < cb) return -1;
        if(ca > cb) return  1;
        if(ca == 0) return  0;
        a++; b++;
    }
}
static void pause_enter(void){
    printf("\nPressione Enter para continuar...");
    char dump[8];
    fgets(dump, sizeof(dump), stdin); 
    fgets(dump, sizeof(dump), stdin); 
}

/* ================== Interface ================== */
static void cabecalho(int usados, int ordenadoPorNome){
    puts("============================");
    puts("PLANO DE FUGA - CODIGO DA ILHA");
    puts("================================");
    printf("Itens na Mochila: %d/%d\n", usados, MAX_ITENS);
    printf("Status da Ordenacao por Nome: %s\n\n",
           ordenadoPorNome ? "ORDENADO" : "NAO ORDENADO");

    puts("1. Adicionar Componente");
    puts("2. Descartar Componente");
    puts("3. Listar Componentes");
    puts("4. Organizar Mochila (Ordenar Componentes)");
    puts("5. Busca Binaria por Componente-Chave (por nome)");
    puts("0. ATIVAR TORRE DE FUGA (Sair)");
    puts("---------------------------------------------");
    printf("Escolha uma opcao: ");
}
static void listar(const Item v[], int n){
    printf("\n------ INVENTARIO ATUAL ------ (%d/%d)\n\n", n, MAX_ITENS);
    puts("-------------------------------------");
    puts("NOME\t\t|TIPO\t\t|QUANTIDADE | PRIORIDADE");
    puts("---------------------------------------------------------");
    for(int i=0;i<n;i++){
        printf("%-16s| %-12s\t| %-9d | %d\n",
               v[i].nome, v[i].tipo, v[i].quantidade, v[i].prioridade);
    }
}


static int adicionar(Item v[], int n){
    if(n >= MAX_ITENS){
        puts("\nMochila cheia! Descarte algo antes de adicionar.");
        return n;
    }
    Item x; char buf[32];

    puts("\n---- Coletando Novo Componente ----");
    printf("Nome:"); ler_linha(x.nome, sizeof(x.nome));
    if(x.nome[0] == '\0'){ puts("Nome invalido. Cancelado."); return n; }

    printf("Tipo:(Estrutural,Eletronico,Energia): "); ler_linha(x.tipo, sizeof(x.tipo));
    if(x.tipo[0] == '\0') strcpy(x.tipo, "desconhecido");

    printf("Quantidade: "); ler_linha(buf, sizeof(buf));
    x.quantidade = atoi(buf); if(x.quantidade < 1) x.quantidade = 1;

    printf("Prioridade de Montagem (1-5): "); ler_linha(buf, sizeof(buf));
    x.prioridade = atoi(buf); if(x.prioridade < 1) x.prioridade = 1; if(x.prioridade > 5) x.prioridade = 5;

    v[n++] = x;
    printf("\nComponente '%s' adicionado!\n", x.nome);
    return n;
}
static int descartar(Item v[], int n){
    if(n == 0){ puts("\nMochila vazia."); return n; }
    char alvo[TAM_NOME];
    printf("\nNome do componente para descartar: "); ler_linha(alvo, sizeof(alvo));
    if(alvo[0] == '\0'){ puts("Nome invalido. Cancelado."); return n; }

    int pos = -1;
    for(int i=0;i<n;i++) if(stricmp_pt(v[i].nome, alvo) == 0){ pos = i; break; }
    if(pos < 0){ printf("'%s' nao encontrado.\n", alvo); return n; }

    for(int i=pos;i<n-1;i++) v[i] = v[i+1];
    n--;
    printf("Componente '%s' descartado.\n", alvo);
    return n;
}

/* ============ Ordenações com contagem ============ */
// 1️ Bubble Sort — compara cada par adjacente
static long ordenarPorNome(Item v[], int n){
    long comps = 0;
    for(int i=0; i<n-1; i++){
        int trocou = 0;
        for(int j=0; j<n-i-1; j++){
            comps++; // comparação entre nomes
            if(stricmp_pt(v[j].nome, v[j+1].nome) > 0){
                Item t = v[j];
                v[j] = v[j+1];
                v[j+1] = t;
                trocou = 1;
            }
        }
        if(!trocou) break; 
    }
    return comps;
}

// 2 Insertion Sort — compara enquanto insere
static long ordenarPorTipo(Item v[], int n){
    long comps = 0;
    for(int i=1; i<n; i++){
        Item chave = v[i];
        int j = i - 1;
        // cada passagem no while é uma comparação
        while(j >= 0){
            comps++;
            if(stricmp_pt(v[j].tipo, chave.tipo) > 0){
                v[j+1] = v[j];
                j--;
            } else break;
        }
        v[j+1] = chave;
    }
    return comps;
}

// Selection Sort — compara cada elemento com o menor
static long ordenarPorPrioridade(Item v[], int n){
    long comps = 0;
    for(int i=0; i<n-1; i++){
        int min_idx = i;
        for(int j=i+1; j<n; j++){
            comps++; // toda verificação é uma comparação
            if(v[j].prioridade < v[min_idx].prioridade){
                min_idx = j;
            }
        }
        if(min_idx != i){
            Item t = v[i];
            v[i] = v[min_idx];
            v[min_idx] = t;
        }
    }
    return comps;
}

/* ============ Busca Binária (requer nome ordenado) ============ */
static int buscaBinariaNome(const Item v[], int n, const char *alvo, long *compsOut){
    int ini=0, fim=n-1; long comps=0;
    while(ini <= fim){
        int mid = (ini+fim)/2;
        comps++;
        int c = stricmp_pt(v[mid].nome, alvo);
        if(c == 0){ if(compsOut) *compsOut = comps; return mid; }
        if(c < 0) ini = mid + 1;
        else      fim = mid - 1;
    }
    if(compsOut) *compsOut = comps;
    return -1;
}

/* ============ Submenu de Organização ============ */
static int submenuOrganizar(Item v[], int n, int *ordenadoPorNome){
    puts("\n---- Estrategia de Organizacao---");
    puts("Como deseja ordenar os componentes?");
    puts("1. Por Nome (Ordem Alfabetica)");
    puts("2. Por Tipo");
    puts("3. Por Prioridade e Montagem");
    puts("0. Cancelar");
    printf("Escolha o criterio: ");
    char buf[16]; ler_linha(buf, sizeof(buf));
    int op = atoi(buf);

    if(op == 0) return 0;

    long comps = 0;
    if(op == 1){
        comps = ordenarPorNome(v, n);
        *ordenadoPorNome = 1;
        puts("\nMochila organizada por NOME.");
    } else if(op == 2){
        comps = ordenarPorTipo(v, n);
        *ordenadoPorNome = 0; 
        puts("\nMochila organizada por TIPO.");
    } else if(op == 3){
        comps = ordenarPorPrioridade(v, n);
        *ordenadoPorNome = 0;
        puts("\nMochila organizada por PRIORIDADE.");
    } else {
        puts("\nOpcao invalida.");
        return 0;
    }

    printf("Analise de Desempenho: Foram necessarias %ld comparacoes.\n", comps);
    listar(v, n);
    return 1;
}

/* ================== Programa ================== */
int main(void){
    Item mochila[MAX_ITENS];
    int usados = 0;
    int ordenadoPorNome = 0; // 0 = NÃO ORDENADO, 1 = ORDENADO por nome

    for(;;){
        cabecalho(usados, ordenadoPorNome);

        char opbuf[16]; ler_linha(opbuf, sizeof(opbuf));
        int op = atoi(opbuf);

        if(op == 0){
            puts("\n*** TORRE DE FUGA ATIVADA! Boa sorte! ***");
            break;
        }

        switch(op){
            case 1: {
                usados = adicionar(mochila, usados);
                ordenadoPorNome = 0;
                listar(mochila, usados);
                pause_enter();
            } break;

            case 2: {
                usados = descartar(mochila, usados);
                ordenadoPorNome = 0; 
                listar(mochila, usados);
                pause_enter();
            } break;

            case 3: {
                listar(mochila, usados);
                pause_enter();
            } break;

            case 4: {
                if(usados == 0){
                    puts("\nMochila vazia.");
                    pause_enter();
                } else {
                    submenuOrganizar(mochila, usados, &ordenadoPorNome);
                    pause_enter();
                }
            } break;

            case 5: {
                if(usados == 0){
                    puts("\nMochila vazia.");
                    pause_enter();
                    break;
                }
                if(!ordenadoPorNome){
                    puts("\nALERTA: A busca binaria requer que a mochila esteja ordenada por NOME.");
                    puts("Use a Opcao 4 para organizar a mochila primeiro.");
                    pause_enter();
                    break;
                }
                char alvo[TAM_NOME];
                printf("\nDigite o nome do componente-chave: ");
                ler_linha(alvo, sizeof(alvo));
                long compsBusca = 0;
                int pos = buscaBinariaNome(mochila, usados, alvo, &compsBusca);
                listar(mochila, usados);
                if(pos >= 0){
                    printf(">> ENCONTRADO em %d: %s | %s | qtd=%d | prio=%d\n",
                           pos, mochila[pos].nome, mochila[pos].tipo,
                           mochila[pos].quantidade, mochila[pos].prioridade);
                } else {
                    printf(">> '%s' NAO encontrado.\n", alvo);
                }
                printf("Comparacoes (busca binaria): %ld\n", compsBusca);
                pause_enter();
            } break;

            default:
                puts("\nOpcao invalida.");
                pause_enter();
        }
    }
    return 0;
}
