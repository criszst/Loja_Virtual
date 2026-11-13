#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 100
#define ARQUIVO "produtos.txt"
#define TEMP "temporario.txt"

// ============================================================
// TOPICO 1 - ESTRUTURA DE DADOS E ENTRADA
// ============================================================
typedef struct {
    int id;
    char nome[50];
    char categoria[50];
    float preco;
    int quantidade;
    char fornecedor[50];
} Produto;

// Le texto com espacos e impede campos vazios ou separador ';'.
void lerTexto(const char *mensagem, char texto[], int tamanho) {
    char linha[200];
    
    while (1) {
        printf("%s", mensagem);
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0);
        }
        if (strchr(linha, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Texto muito longo.\n");
            continue;
        }
        linha[strcspn(linha, "\n")] = '\0';
        if (linha[0] == '\0' || strlen(linha) >= (size_t)tamanho ||
            strchr(linha, ';') != NULL || strchr(linha, '"') != NULL) {
            printf("Texto invalido (maximo %d caracteres, sem ; ou aspas).\n", tamanho - 1);
            continue;
        }
        strcpy(texto, linha);
        return;
    }
}

int lerInteiro(const char *mensagem, int minimo) {
    char linha[100], extra;
    int numero;
    while (1) {
        printf("%s", mensagem);
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0);
        }
        if (sscanf(linha, " %d %c", &numero, &extra) == 1 && numero >= minimo)
            return numero;
        printf("Digite um numero inteiro valido.\n");
    }
}

float lerPreco(void) {
    char linha[100], extra;
    float preco;
    while (1) {
        printf("Preco (ex.: 49.90): ");
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0);
        }
        if (sscanf(linha, " %f %c", &preco, &extra) == 1 &&
            preco >= 0 && preco <= 1000000) return preco;
        printf("Preco invalido. Use ponto decimal.\n");
    }
}

// Compara letras sem diferenciar maiusculas e minusculas (ASCII).
int comparar(const char *a, const char *b) {
    while (*a && *b) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0) {
            return d;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

// ============================================================
// TOPICO 2 - ARQUIVOS E IDENTIFICADORES
// ============================================================
int carregar(Produto lista[]) {
    FILE *arquivo = fopen(ARQUIVO, "r");
    int total = 0;
    char linha[300];
    if (arquivo == NULL) {
        return 0; // Cadastro ainda nao criado.
    }
    while (total < MAX && fgets(linha, sizeof(linha), arquivo)) {
        Produto p;
        if (sscanf(linha, "%d;%49[^;];%49[^;];%f;%d;%49[^\n]",
                   &p.id, p.nome, p.categoria, &p.preco,
                   &p.quantidade, p.fornecedor) == 6)
            lista[total++] = p;
    }
    fclose(arquivo);
    return total;
}

// Grava em arquivo temporario e substitui o original.
int salvar(Produto lista[], int total) {
    FILE *arquivo = fopen(TEMP, "w");
    if (arquivo == NULL) {
        perror("Erro ao criar temporario");
        return 0;
    }
    for (int i = 0; i < total; i++) {
        Produto *p = &lista[i]; // Ponteiro para uma estrutura.
        if (fprintf(arquivo, "%d;%s;%s;%.2f;%d;%s\n", p->id, p->nome,
                    p->categoria, p->preco, p->quantidade, p->fornecedor) < 0) {
            fclose(arquivo);
            remove(TEMP);
            return 0;
        }
    }
    if (fclose(arquivo) != 0) {
        remove(TEMP);
        return 0;
    }
    // Em alguns sistemas, rename nao substitui arquivo existente.
    if (rename(TEMP, ARQUIVO) == 0) {
        return 1;
    }
    if (rename(ARQUIVO, "produtos_backup.txt") != 0) {
        return 0;
    }
    if (rename(TEMP, ARQUIVO) == 0) {
        remove("produtos_backup.txt");
        return 1;
    }
    rename("produtos_backup.txt", ARQUIVO);
    return 0;
}

// Guarda o maior ID usado, mesmo se o produto for excluido.
int proximoId(Produto lista[], int total) {
    int maior = 0;
    FILE *arquivo = fopen("ultimo_id.txt", "r");
    if (arquivo != NULL) {
        if (fscanf(arquivo, "%d", &maior) != 1) maior = 0;
        fclose(arquivo);
    }
    for (int i = 0; i < total; i++)
        if (lista[i].id > maior) {
            maior = lista[i].id;
        }
    return maior + 1;
}

void registrarId(int id) {
    FILE *arquivo = fopen("ultimo_id.txt", "w");
    if (arquivo != NULL) {
        fprintf(arquivo, "%d\n", id);
        fclose(arquivo);
    }
}

// ============================================================
// TOPICO 3 - LISTAGEM, BUSCA E CADASTRO
// ============================================================
void linhaTabela(void) {
    printf("-------------------------------------------------------------------------------------------------------------\n");
}

void mostrarProduto(Produto *p) {
    printf("| %-3d | %-20.20s | %-18.18s | %10.2f | %10d | %-20.20s |\n",
           p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
}

void cabecalho(void) {
    linhaTabela();
    printf("| %-3s | %-20s | %-18s | %-10s | %-10s | %-20s |\n",
           "ID", "Produto", "Categoria", "Preco(R$)", "Quantidade", "Fornecedor");
    linhaTabela();
}

void listar(Produto lista[], int total) {
    if (total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }
    cabecalho();
    for (int i = 0; i < total; i++) {
        mostrarProduto(&lista[i]);
    }
    linhaTabela();
}

int indicePorId(Produto lista[], int total, int id) {
    for (int i = 0; i < total; i++)
        if (lista[i].id == id) {
            return i;
        }
    return -1;
}

int nomeRepetido(Produto lista[], int total, const char *nome, int ignorarId) {
    for (int i = 0; i < total; i++)
        if (lista[i].id != ignorarId && comparar(lista[i].nome, nome) == 0) {
            return 1;
        }
    return 0;
}

void cadastrar(Produto lista[], int *total) {
    if (*total >= MAX) {
        printf("Limite de produtos atingido.\n");
        return;
    }
    Produto p;
    lerTexto("Produto: ", p.nome, 50);
    if (nomeRepetido(lista, *total, p.nome, -1)) {
        printf("Produto com esse nome ja cadastrado.\n");
        return;
    }
    lerTexto("Categoria: ", p.categoria, 50);
    p.preco = lerPreco();
    p.quantidade = lerInteiro("Quantidade: ", 0);
    lerTexto("Fornecedor: ", p.fornecedor, 50);
    p.id = proximoId(lista, *total);
    if (indicePorId(lista, *total, p.id) != -1) {
        printf("Erro: ID duplicado.\n");
        return;
    }
    lista[*total] = p;
    (*total)++;
    if (salvar(lista, *total)) {
        registrarId(p.id);
        printf("Produto cadastrado! ID: %d\n", p.id);
    } else {
        (*total)--;
        printf("Nao foi possivel salvar.\n");
    }
}

void buscar(Produto lista[], int total, int categoria) {
    char texto[50];
    int encontrados = 0;
    lerTexto(categoria ? "Categoria: " : "Nome: ", texto, 50);
    for (int i = 0; i < total; i++) {
        const char *campo = categoria ? lista[i].categoria : lista[i].nome;
        if (comparar(campo, texto) == 0) {
            if (encontrados == 0) {
                cabecalho();
            }
            mostrarProduto(&lista[i]);
            encontrados++;
        }
    }
    if (encontrados > 0) {
        linhaTabela();
    } else {
        printf("Nenhum produto encontrado.\n");
    }
}

// ============================================================
// TOPICO 4 - ALTERACAO E EXCLUSAO
// ============================================================
void alterar(Produto lista[], int total) {
    int id = lerInteiro("ID para alterar: ", 1);
    int i = indicePorId(lista, total, id);
    if (i == -1) {
        printf("Produto nao encontrado.\n");
        return;
    }
    Produto antigo = lista[i]; // Copia para desfazer se houver erro.
    Produto *p = &lista[i];
    lerTexto("Novo nome: ", p->nome, 50);
    if (nomeRepetido(lista, total, p->nome, id)) {
        *p = antigo;
        printf("Nome ja utilizado.\n");
        return;
    }
    lerTexto("Nova categoria: ", p->categoria, 50);
    p->preco = lerPreco();
    p->quantidade = lerInteiro("Nova quantidade: ", 0);
    lerTexto("Novo fornecedor: ", p->fornecedor, 50);
    if (salvar(lista, total)) {
        printf("Produto alterado.\n");
    } else {
        *p = antigo;
        printf("Erro ao alterar.\n");
    }
}

void excluir(Produto lista[], int *total) {
    int id = lerInteiro("ID para excluir: ", 1);
    int i = indicePorId(lista, *total, id);
    if (i == -1) {
        printf("Produto nao encontrado.\n");
        return;
    }
    Produto removido = lista[i];
    for (int j = i; j < *total - 1; j++) {
        lista[j] = lista[j + 1];
    }
    (*total)--;
    if (salvar(lista, *total)) {
        printf("Produto excluido.\n");
    } else {
        for (int j = *total; j > i; j--) {
            lista[j] = lista[j - 1];
        }
        lista[i] = removido;
        (*total)++;
        printf("Erro ao excluir.\n");
    }
}

// ============================================================
// TOPICO 5 - BUBBLE SORT E PONTEIROS
// ============================================================
void trocar(Produto *a, Produto *b) {
    Produto auxiliar = *a;
    *a = *b;
    *b = auxiliar;
}

void ordenar(Produto lista[], int total) {
    if (total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }
    printf("\n1 - Nome A-Z\n");
    printf("2 - Nome Z-A\n");
    printf("3 - Menor preco\n");
    printf("4 - Maior preco\n");
    int opcao = lerInteiro("Opcao: ", 1);
    if (opcao > 4) {
        printf("Opcao invalida.\n");
        return;
    }
    Produto copia[MAX]; // Ordena uma copia, sem alterar arquivo original.
    for (int i = 0; i < total; i++) {
        copia[i] = lista[i];
    }
    for (int i = 0; i < total - 1; i++) {
        for (int j = 0; j < total - i - 1; j++) {
            int ordem = comparar(copia[j].nome, copia[j + 1].nome);
            int mudar = (opcao == 1 && ordem > 0) ||
                        (opcao == 2 && ordem < 0) ||
                        (opcao == 3 && copia[j].preco > copia[j + 1].preco) ||
                        (opcao == 4 && copia[j].preco < copia[j + 1].preco);
            if (mudar) {
                trocar(&copia[j], &copia[j + 1]);
            }
        }
    }
    listar(copia, total);
}

// ============================================================
// TOPICO 6 - EXPORTACAO CSV E MENU
// ============================================================
void exportarCSV(Produto lista[], int total) {
    FILE *arquivo = fopen("produtos.csv", "w");
    if (arquivo == NULL) {
        perror("Erro ao exportar CSV");
        return;
    }
    fprintf(arquivo, "ID,Produto,Categoria,Preco,Quantidade,Fornecedor\n");
    for (int i = 0; i < total; i++) {
        Produto *p = &lista[i];
        fprintf(arquivo, "%d,\"%s\",\"%s\",%.2f,%d,\"%s\"\n",
                p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
    }
    if (fclose(arquivo) == 0) {
        printf("Arquivo produtos.csv criado.\n");
    } else {
        printf("Erro ao finalizar CSV.\n");
    }
}

int main(void) {
    Produto lista[MAX];
    int total = carregar(lista); // Se nao existir arquivo, comeca vazio.
    int opcao;
    do {
        printf("\n========== MENU PRINCIPAL ==========\n");
        printf("1 - Cadastrar produto\n");
        printf("2 - Listar produtos\n");
        printf("3 - Alterar produto\n");
        printf("4 - Excluir produto\n");
        printf("5 - Buscar por nome\n");
        printf("6 - Buscar por categoria\n");
        printf("7 - Ordenar produtos\n");
        printf("8 - Exportar CSV\n");
        printf("0 - Sair\n");
        opcao = lerInteiro("Opcao: ", 0);
        switch (opcao) {
            case 1:
                cadastrar(lista, &total);
                break;
            case 2:
                listar(lista, total);
                break;
            case 3:
                alterar(lista, total);
                break;
            case 4:
                excluir(lista, &total);
                break;
            case 5:
                buscar(lista, total, 0);
                break;
            case 6:
                buscar(lista, total, 1);
                break;
            case 7:
                ordenar(lista, total);
                break;
            case 8:
                exportarCSV(lista, total);
                break;
            case 0:
                printf("Encerrando programa.\n");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (opcao != 0);
    return 0;
}
