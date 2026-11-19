
#include <stdio.h>   // Funções de entrada e saída: printf, fgets, fopen, fprintf etc.
#include <stdlib.h>  // Funções gerais, como exit().
#include <string.h>  // Manipulação de strings: strlen, strcpy, strchr etc.
#include <ctype.h>   // Manipulação de caracteres: tolower().

// Define constantes utilizadas durante o programa.
#define MAX 100                    // Quantidade máxima de produtos.
#define ARQUIVO "produtos.txt"     // Arquivo principal de armazenamento.
#define TEMP "temporario.txt"     // Arquivo temporário usado ao salvar.

// ============================================================
// TOPICO 1 - ESTRUTURA DE DADOS E ENTRADA
// ============================================================

// Cria um tipo de dado chamado Produto.
// A struct agrupa informações relacionadas em uma única estrutura.
typedef struct {
    int id;                // Identificador único do produto.
    char nome[50];         // Nome do produto (até 49 caracteres + '\0').
    char categoria[50];    // Categoria do produto.
    float preco;           // Preço do produto.
    int quantidade;        // Quantidade disponível em estoque.
    char fornecedor[50];   // Nome do fornecedor.
} Produto;


// Lê textos digitados pelo usuário.
// Impede textos vazios, muito longos ou com caracteres proibidos.
//
// Parâmetros:
// mensagem: texto exibido ao usuário.
// texto[]: variável onde a resposta será armazenada.
// tamanho: capacidade máxima do campo.
void lerTexto(const char *mensagem, char texto[], int tamanho) {

    // Buffer temporário para receber a entrada do usuário.
    char linha[200];

    // Repete até o usuário digitar um texto válido.
    while (1) {

        // Exibe a mensagem solicitando a informação.
        printf("%s", mensagem);

        // Lê a entrada, incluindo espaços.
        // fgets evita ultrapassar o tamanho do buffer.
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0); // Encerra se a entrada não estiver disponível.
        }

        // Verifica se a entrada possui '\n'.
        // Se não possuir, pode ter ultrapassado o buffer.
        if (strchr(linha, '\n') == NULL) {

            int c;

            // Descarta os caracteres restantes da entrada.
            while ((c = getchar()) != '\n' && c != EOF) {}

            printf("Texto muito longo.\n");

            // Reinicia o laço e solicita novamente.
            continue;
        }

        // Localiza o '\n' e substitui por '\0'.
        // '\0' indica o final de uma string em C.
        linha[strcspn(linha, "\n")] = '\0';

        // Valida o conteúdo digitado.
        if (
            linha[0] == '\0' ||                  // Não aceita texto vazio.
            strlen(linha) >= (size_t)tamanho || // Não aceita texto grande demais.
            strchr(linha, ';') != NULL ||        // Não aceita ponto e vírgula.
            strchr(linha, '"') != NULL           // Não aceita aspas.
        ) {
            printf(
                "Texto invalido (maximo %d caracteres, sem ; ou aspas).\n",
                tamanho - 1
            );

            continue;
        }

        // Copia o texto validado para a variável de destino.
        strcpy(texto, linha);

        // Finaliza a função após uma entrada válida.
        return;
    }
}


// Lê um número inteiro e verifica se respeita o valor mínimo.
int lerInteiro(const char *mensagem, int minimo) {

    char linha[100]; // Armazena a entrada como texto.
    char extra;      // Detecta caracteres extras após o número.
    int numero;      // Armazena o inteiro convertido.

    while (1) {

        printf("%s", mensagem);

        // Recebe a entrada do usuário.
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0);
        }

        // sscanf tenta converter a string para inteiro.
        //
        // %d: lê um número inteiro.
        // %c: tenta identificar caracteres adicionais.
        //
        // == 1 significa que somente o inteiro foi convertido.
        // numero >= minimo impede valores abaixo do permitido.
        if (
            sscanf(linha, " %d %c", &numero, &extra) == 1 &&
            numero >= minimo
        ) {
            return numero;
        }

        printf("Digite um numero inteiro valido.\n");
    }
}


// Lê e valida o preço de um produto.
float lerPreco(void) {

    char linha[100];
    char extra;
    float preco;

    while (1) {

        printf("Preco (ex.: 49.90): ");

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            exit(0);
        }

        // %f converte o texto para float.
        // &preco fornece o endereço onde o valor será armazenado.
        //
        // Verifica se:
        // 1. A entrada contém um número válido.
        // 2. Não existem caracteres extras.
        // 3. O preço está entre 0 e 1.000.000.
        if (
            sscanf(linha, " %f %c", &preco, &extra) == 1 &&
            preco >= 0 &&
            preco <= 1000000
        ) {
            return preco;
        }

        printf("Preco invalido. Use ponto decimal.\n");
    }
}


// Compara duas strings ignorando diferenças entre letras
// maiúsculas e minúsculas, considerando caracteres ASCII.
//
// Retorna:
// 0: strings iguais.
// Negativo: primeira string vem antes da segunda.
// Positivo: primeira string vem depois da segunda.
int comparar(const char *a, const char *b) {

    // Percorre as strings enquanto ambas tiverem caracteres.
    // *a e *b representam os caracteres nas posições atuais.
    while (*a && *b) {

        // Converte os caracteres para minúsculos e calcula
        // a diferença entre seus valores numéricos.
        int d = tolower((unsigned char)*a)
              - tolower((unsigned char)*b);

        // Se houver diferença, as strings não são iguais.
        if (d != 0) {
            return d;
        }

        // Avança os ponteiros para os próximos caracteres.
        a++;
        b++;
    }

    // Compara os caracteres finais.
    // Também identifica strings com tamanhos diferentes.
    return tolower((unsigned char)*a)
         - tolower((unsigned char)*b);
}


// ============================================================
// TOPICO 2 - ARQUIVOS E IDENTIFICADORES
// ============================================================


// Carrega os produtos salvos no arquivo para a memória.
//
// lista[]: vetor que receberá os produtos.
// Retorno: quantidade de produtos carregados.
int carregar(Produto lista[]) {

    // Abre o arquivo principal no modo leitura ("r").
    FILE *arquivo = fopen(ARQUIVO, "r");

    int total = 0;   // Contador de produtos carregados.
    char linha[300]; // Armazena cada linha lida do arquivo.

    // Se o arquivo não existir, começa com cadastro vazio.
    if (arquivo == NULL) {
        return 0;
    }

    // Lê o arquivo linha por linha.
    //
    // total < MAX: impede ultrapassar o limite do vetor.
    // fgets: lê uma linha do arquivo.
    while (
        total < MAX &&
        fgets(linha, sizeof(linha), arquivo)
    ) {

        // Estrutura temporária para armazenar os dados lidos.
        Produto p;

        // sscanf separa os campos da linha utilizando ';'.
        //
        // %d: inteiro.
        // %49[^;]: lê até encontrar ';', com limite de 49 caracteres.
        // %f: número decimal.
        // %49[^\n]: lê até o final da linha.
        //
        // == 6 indica que os seis campos foram convertidos.
        if (
            sscanf(
                linha,
                "%d;%49[^;];%49[^;];%f;%d;%49[^\n]",
                &p.id,
                p.nome,
                p.categoria,
                &p.preco,
                &p.quantidade,
                p.fornecedor
            ) == 6
        ) {

            // Armazena o produto na próxima posição do vetor.
            lista[total] = p;

            // Incrementa a quantidade de produtos.
            total++;
        }
    }

    // Fecha o arquivo após a leitura.
    fclose(arquivo);

    // Retorna a quantidade de produtos carregados.
    return total;
}


// Salva todos os produtos em um arquivo.
//
// Utiliza um arquivo temporário para evitar sobrescrever
// diretamente o arquivo principal durante a gravação.
//
// Retorna:
// 1: sucesso.
// 0: erro.
int salvar(Produto lista[], int total) {

    // Cria ou sobrescreve o arquivo temporário.
    // "w" representa o modo escrita.
    FILE *arquivo = fopen(TEMP, "w");

    // Verifica se foi possível abrir o arquivo.
    if (arquivo == NULL) {

        // Exibe uma mensagem de erro do sistema.
        perror("Erro ao criar temporario");

        return 0;
    }

    // Percorre todos os produtos cadastrados.
    for (int i = 0; i < total; i++) {

        // Ponteiro para o produto da posição atual.
        // &lista[i] obtém o endereço da estrutura.
        Produto *p = &lista[i];

        // fprintf escreve os dados no arquivo.
        //
        // -> acessa atributos por meio de um ponteiro.
        // %.2f grava o preço com duas casas decimais.
        // ';' separa os campos.
        if (
            fprintf(
                arquivo,
                "%d;%s;%s;%.2f;%d;%s\n",
                p->id,
                p->nome,
                p->categoria,
                p->preco,
                p->quantidade,
                p->fornecedor
            ) < 0
        ) {

            // Se a gravação falhar, fecha e remove o temporário.
            fclose(arquivo);
            remove(TEMP);

            return 0;
        }
    }

    // Fecha o arquivo e verifica se houve erro.
    if (fclose(arquivo) != 0) {

        remove(TEMP);

        return 0;
    }

    // Tenta renomear o temporário para o arquivo principal.
    // rename retorna 0 quando a operação funciona.
    if (rename(TEMP, ARQUIVO) == 0) {
        return 1;
    }

    // Caso a substituição direta falhe, tenta mover
    // o arquivo original para um arquivo de backup.
    if (rename(ARQUIVO, "produtos_backup.txt") != 0) {
        return 0;
    }

    // Tenta novamente transformar o temporário no principal.
    if (rename(TEMP, ARQUIVO) == 0) {

        // Remove o backup após salvar com sucesso.
        remove("produtos_backup.txt");

        return 1;
    }

    // Se a segunda tentativa falhar, restaura o original.
    rename("produtos_backup.txt", ARQUIVO);

    return 0;
}


// Descobre o próximo ID disponível.
//
// Utiliza o arquivo ultimo_id.txt para evitar reutilizar
// identificadores de produtos excluídos.
int proximoId(Produto lista[], int total) {

    int maior = 0;

    // Abre o arquivo que armazena o último ID utilizado.
    FILE *arquivo = fopen("ultimo_id.txt", "r");

    if (arquivo != NULL) {

        // Lê o último ID salvo.
        // Se a leitura falhar, considera 0.
        if (fscanf(arquivo, "%d", &maior) != 1) {
            maior = 0;
        }

        fclose(arquivo);
    }

    // Percorre os produtos para encontrar o maior ID atual.
    for (int i = 0; i < total; i++) {

        if (lista[i].id > maior) {
            maior = lista[i].id;
        }
    }

    // O novo identificador será o maior ID + 1.
    return maior + 1;
}


// Registra o último ID utilizado em um arquivo separado.
void registrarId(int id) {

    // Abre o arquivo no modo escrita.
    FILE *arquivo = fopen("ultimo_id.txt", "w");

    if (arquivo != NULL) {

        // Grava o identificador.
        fprintf(arquivo, "%d\n", id);

        // Fecha o arquivo.
        fclose(arquivo);
    }
}


// ============================================================
// TOPICO 3 - LISTAGEM, BUSCA E CADASTRO
// ============================================================


// Imprime uma linha horizontal para formatar a tabela.
void linhaTabela(void) {

    printf("-------------------------------------------------------------------------------------------------------------\n");
}


// Exibe os dados de um único produto.
//
// Recebe um ponteiro para evitar copiar toda a estrutura.
void mostrarProduto(Produto *p) {

    // %-20.20s: texto alinhado à esquerda, com largura 20.
    // %10.2f: número decimal alinhado à direita, com 2 casas.
    // -> acessa os campos da estrutura apontada por p.
    printf(
        "| %-3d | %-20.20s | %-18.18s | %10.2f | %10d | %-20.20s |\n",
        p->id,
        p->nome,
        p->categoria,
        p->preco,
        p->quantidade,
        p->fornecedor
    );
}


// Exibe o cabeçalho da tabela.
void cabecalho(void) {

    linhaTabela();

    printf(
        "| %-3s | %-20s | %-18s | %-10s | %-10s | %-20s |\n",
        "ID",
        "Produto",
        "Categoria",
        "Preco(R$)",
        "Quantidade",
        "Fornecedor"
    );

    linhaTabela();
}


// Lista todos os produtos cadastrados.
void listar(Produto lista[], int total) {

    // Verifica se o vetor está vazio.
    if (total == 0) {

        printf("Nenhum produto cadastrado.\n");

        return;
    }

    // Imprime o cabeçalho.
    cabecalho();

    // Percorre o vetor e exibe cada produto.
    for (int i = 0; i < total; i++) {

        // &lista[i] passa o endereço do produto.
        mostrarProduto(&lista[i]);
    }

    linhaTabela();
}


// Procura um produto pelo ID.
//
// Retorna o índice do produto no vetor.
// Retorna -1 quando não encontra.
int indicePorId(Produto lista[], int total, int id) {

    // Busca sequencial: verifica cada produto.
    for (int i = 0; i < total; i++) {

        // Compara o ID atual com o ID procurado.
        if (lista[i].id == id) {
            return i;
        }
    }

    // -1 indica que nenhum produto foi encontrado.
    return -1;
}


// Verifica se já existe um produto com o mesmo nome.
//
// ignorarId permite desconsiderar um produto específico,
// principalmente durante a alteração de cadastro.
int nomeRepetido(
    Produto lista[],
    int total,
    const char *nome,
    int ignorarId
) {

    for (int i = 0; i < total; i++) {

        // Verifica duas condições:
        // 1. O produto não é aquele que devemos ignorar.
        // 2. O nome é igual ao nome pesquisado.
        if (
            lista[i].id != ignorarId &&
            comparar(lista[i].nome, nome) == 0
        ) {
            return 1; // Nome já utilizado.
        }
    }

    return 0; // Nome disponível.
}


// Cadastra um novo produto.
//
// int *total é um ponteiro porque a função precisa
// modificar a quantidade de produtos da função main.
void cadastrar(Produto lista[], int *total) {

    // *total acessa o valor armazenado no endereço recebido.
    if (*total >= MAX) {

        printf("Limite de produtos atingido.\n");

        return;
    }

    // Cria uma estrutura temporária para o novo produto.
    Produto p;

    // Solicita o nome.
    lerTexto("Produto: ", p.nome, 50);

    // Verifica se já existe um produto com esse nome.
    // -1 significa que nenhum ID deve ser ignorado.
    if (nomeRepetido(lista, *total, p.nome, -1)) {

        printf("Produto com esse nome ja cadastrado.\n");

        return;
    }

    // Solicita as demais informações.
    lerTexto("Categoria: ", p.categoria, 50);

    // Atribui o retorno da função ao campo preco.
    p.preco = lerPreco();

    // A quantidade mínima permitida é zero.
    p.quantidade = lerInteiro("Quantidade: ", 0);

    lerTexto("Fornecedor: ", p.fornecedor, 50);

    // Gera automaticamente um novo identificador.
    p.id = proximoId(lista, *total);

    // Verificação adicional contra IDs duplicados.
    if (indicePorId(lista, *total, p.id) != -1) {

        printf("Erro: ID duplicado.\n");

        return;
    }

    // Insere o novo produto no final do vetor.
    lista[*total] = p;

    // Incrementa o contador original.
    (*total)++;

    // Tenta salvar os produtos no arquivo.
    if (salvar(lista, *total)) {

        // Registra o último ID utilizado.
        registrarId(p.id);

        printf("Produto cadastrado! ID: %d\n", p.id);

    } else {

        // Desfaz o incremento se a gravação falhar.
        (*total)--;

        printf("Nao foi possivel salvar.\n");
    }
}


// Busca produtos pelo nome ou categoria.
//
// categoria == 0: busca por nome.
// categoria != 0: busca por categoria.
void buscar(Produto lista[], int total, int categoria) {

    char texto[50];

    // Contador de resultados encontrados.
    int encontrados = 0;

    // Operador ternário:
// condição ? valor_se_verdadeiro : valor_se_falso
    lerTexto(
        categoria ? "Categoria: " : "Nome: ",
        texto,
        50
    );

    // Percorre todos os produtos.
    for (int i = 0; i < total; i++) {

        // Escolhe o campo utilizado na comparação.
        const char *campo = categoria
            ? lista[i].categoria
            : lista[i].nome;

        // Compara o campo com o texto informado.
        if (comparar(campo, texto) == 0) {

            // Imprime o cabeçalho apenas no primeiro resultado.
            if (encontrados == 0) {
                cabecalho();
            }

            mostrarProduto(&lista[i]);

            encontrados++;
        }
    }

    // Verifica se houve resultados.
    if (encontrados > 0) {

        linhaTabela();

    } else {

        printf("Nenhum produto encontrado.\n");
    }
}


// ============================================================
// TOPICO 4 - ALTERACAO E EXCLUSAO
// ============================================================


// Altera as informações de um produto existente.
void alterar(Produto lista[], int total) {

    // Solicita o ID do produto.
    int id = lerInteiro("ID para alterar: ", 1);

    // Procura a posição do produto no vetor.
    int i = indicePorId(lista, total, id);

    // Verifica se o produto existe.
    if (i == -1) {

        printf("Produto nao encontrado.\n");

        return;
    }

    // Guarda uma cópia do produto original.
    // Isso permite desfazer alterações em caso de erro.
    Produto antigo = lista[i];

    // Ponteiro para o produto que será alterado.
    Produto *p = &lista[i];

    // Modifica diretamente o nome na estrutura original.
    lerTexto("Novo nome: ", p->nome, 50);

    // Verifica duplicidade, ignorando o próprio produto.
    if (nomeRepetido(lista, total, p->nome, id)) {

        // Restaura o produto original.
        *p = antigo;

        printf("Nome ja utilizado.\n");

        return;
    }

    // Atualiza os demais campos.
    lerTexto("Nova categoria: ", p->categoria, 50);

    p->preco = lerPreco();

    p->quantidade = lerInteiro("Nova quantidade: ", 0);

    lerTexto("Novo fornecedor: ", p->fornecedor, 50);

    // Salva o vetor atualizado.
    if (salvar(lista, total)) {

        printf("Produto alterado.\n");

    } else {

        // Desfaz todas as alterações em memória.
        *p = antigo;

        printf("Erro ao alterar.\n");
    }
}


// Exclui um produto do vetor e atualiza o arquivo.
void excluir(Produto lista[], int *total) {

    // Solicita o identificador do produto.
    int id = lerInteiro("ID para excluir: ", 1);

    // Busca o índice correspondente.
    int i = indicePorId(lista, *total, id);

    if (i == -1) {

        printf("Produto nao encontrado.\n");

        return;
    }

    // Guarda o produto removido para possível restauração.
    Produto removido = lista[i];

    // Desloca os elementos seguintes uma posição à esquerda.
    // Isso elimina o espaço ocupado pelo produto excluído.
    for (int j = i; j < *total - 1; j++) {

        lista[j] = lista[j + 1];
    }

    // Reduz a quantidade de produtos.
    (*total)--;

    // Tenta salvar o vetor atualizado.
    if (salvar(lista, *total)) {

        printf("Produto excluido.\n");

    } else {

        // Se a gravação falhar, desloca os elementos
        // novamente para a direita.
        for (int j = *total; j > i; j--) {

            lista[j] = lista[j - 1];
        }

        // Restaura o produto removido.
        lista[i] = removido;

        // Restaura a quantidade anterior.
        (*total)++;

        printf("Erro ao excluir.\n");
    }
}


// ==========================================================
