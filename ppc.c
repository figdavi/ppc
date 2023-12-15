#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ENDERECO_BIBLIOTECAS "C:\\MinGW\\include\\"

typedef struct listad {
  char *nomeDefine;
  char *valorDefine;
  struct listad *prox;
} ListaD;

ListaD *listaDefine = NULL;

typedef struct listai {
  char *nomeInclude;
  struct listai *prox;
} ListaI;

ListaI *listaInclude = NULL;

int isalnum_(char c);
int palavraReservada(char *palavra);
void escreverStrings(FILE *input, FILE *output, char delimitador);
void escreverDefinesIncludes(FILE *input, FILE *output);
void removerComentarios(FILE *input, FILE *output, char c);
void identificarComentarios(char *inputName, char *outputName);
int precisaEspaco(FILE *input, FILE *output, char *palavra);
void removerEspacos(char *inputName, char *outputName);
void addDefine(char *linha);
int isDefine(char *palavra, char **returnValorDefine);
void expandirDefines(char *inputName, char *outputName);
void addInclude(char *linha);
void expandirIncludes(char *output);
void contDefinesIncludes(char *inputName);



int isalnum_(char c) {
  return (isalnum(c) || c == '_');
}

int palavraReservada(char *palavra) {
  char *palavrasRes[] = {"struct", "void", "short", "long", "int", "char", "float", "return", "typedef", "FILE", "EOF", "else", "case"};
  int i, size = sizeof(palavrasRes) / sizeof(palavrasRes[0]);
  
  for(i = 0; i < size; i++) {
    if(strcmp(palavra, palavrasRes[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

void escreverStrings(FILE *input, FILE *output, char delimitador) { 
  // Aspas simples ou duplas
  char c = delimitador; 

  do {
    if(c == '\\') {
      putc(c, output);
      c = getc(input);
    }
    putc(c, output);
  } while((c = getc(input)) != delimitador);
  putc(c, output);
}

void escreverDefinesIncludes(FILE *input, FILE *output) {
  char palavra[200];

  putc('#', output);

  fgets(palavra, sizeof(palavra), input);
  fputs(palavra, output);
}


void identificarComentarios(char *inputName, char *outputName) {
  FILE *input = fopen(inputName, "r");
  FILE *output = fopen(outputName, "w");
  if (input == NULL || output == NULL) {
    printf("Erro ao abrir os arquivos.\n");
    exit(1);
  }

  int i;
  char c;

  while ((c = getc(input)) != EOF) {
    switch (c) {

      case '/':
        removerComentarios(input, output, c);
        break;

      case '\"':
      case '\'':
        escreverStrings(input, output, c);
        break;

      default:
        putc(c, output);
        break;
      }
  }

  fclose(input);
  fclose(output);
}

void removerComentarios(FILE *input, FILE *output, char c) {

  char proximo = getc(input);

  if (proximo == '/') { //
    while ((c = getc(input)) != '\n'){}

    putc(c, output); //Colocar \n

  } else if (proximo == '*') { /**/
    while ((c = getc(input)) != EOF) {
      if(c == '*' && (proximo = getc(input)) == '/') {
        break;
      }
    }

  } else { //Não é comentário
    putc(c, output);
    ungetc(proximo, input);
  }
}



void removerEspacos(char *inputName, char *outputName) {
  FILE *input = fopen(inputName, "r");
  FILE *output = fopen(outputName, "w");
  if (input == NULL || output == NULL) {
    printf("Erro ao abrir os arquivos.\n");
    exit(1);
  }

  char c, palavra[1000];
  int i = 0;

  while((c = getc(input)) != EOF) {
    switch(c) {
      case '\"':
      case '\'':
        escreverStrings(input, output, c);
        break;

      case '#':
        escreverDefinesIncludes(input, output);
        break;

      case ' ':
      case '\t':
      case '\n':
      case '\r':
      case '\v':
      case '\f':
        break;

      default:

        // Acumular palavras reservadas
        palavra[i] = c;
        palavra[++i] = '\0';

        // Não existe palavra reservada com charEspeciais ()
        if(!isalnum_(c)) { 
          fputs(palavra, output);
          i = 0;
          continue;
        }

        if(palavraReservada(palavra)) {
          fputs(palavra, output);
          i = 0;

          if(precisaEspaco(input, output, palavra)) {
            putc(' ', output);
          }
        }
        break;

    }
  }

  fclose(input);
  fclose(output);
}

int precisaEspaco(FILE *input, FILE *output, char *palavra) {

  char c, proximo;

  // Em situacoes de case"something": , " é escrito antes, por isso
  if(strcmp(palavra, "case") == 0) {
    do {
      if(!isspace(c = getc(input))) {
        if(isalnum_(c)) { //Se o primeiro caractere não é um ' ou ", necessita espaço
          ungetc(c, input);
          return 1;
        }
        ungetc(c, input);
        return 0;
      }
    } while(c != ':');
  }
  
  if(strcmp(palavra, "else") == 0) {
    c = getc(input);
    proximo = getc(input);

    if(isspace(c) && isalnum_(proximo)) { //"else if", "else j++"
      ungetc(proximo, input);
      return 1;
    } else if(isspace(c) && !isalnum_(proximo)){
      ungetc(proximo, input);
      return 0;
    } else {
      //Why does this work
      ungetc(proximo, input);
      ungetc(c, input);
      return 0;
    }
  }


  if(isspace(c = getc(input))) { // Se temos palavra res e espaço logo depois
    return 1;

  } else { // Nomes que possue palavras reservadas junto como "intervalo"
    ungetc(c, input);
    return 0;
  }
}


void contDefinesIncludes(char *inputName) {
  FILE *input = fopen(inputName, "r");

  if (input == NULL) {
    printf("Erro ao abrir os arquivos.\n");
    exit(1);
  }
  
  char linha[300];

  while(fgets(linha, sizeof(linha), input) != NULL) {


    //Se os primeiros strlen("#include")'caracteres sao "#include"
    if (strncmp(linha, "#include", strlen("#include")) == 0) {

      addInclude(linha);

    } else if (strncmp(linha, "#define", strlen("#define")) == 0) {

      addDefine(linha);

    }
  }

  fclose(input);
}


void addInclude(char *linha) {
  char nomeArquivo[50], nomeAux[20];

  //Dois tipos
  if(strstr(linha, "\"")) { 
    sscanf(linha, "#include \"%[^\"]\"", nomeArquivo); 

  } else {
    sscanf(linha, "#include <%[^>]>", nomeAux);

    strcpy(nomeArquivo, ENDERECO_BIBLIOTECAS);
    strcat(nomeArquivo, nomeAux);
  }
  

  ListaI *no = malloc(sizeof(ListaI));

  no->nomeInclude = malloc(strlen(nomeArquivo) + 1);
	strcpy(no->nomeInclude, nomeArquivo);
	no->prox = NULL;
	
	ListaI *aux = listaInclude;
  if(aux == NULL) {
    listaInclude = no;
    
  } else {
    while(aux->prox != NULL) {
		  aux = aux->prox;
	  }
	  aux->prox = no;
  }
}

void expandirIncludes(char *outputName) {
  FILE *output = fopen(outputName, "w");
  if (output == NULL) {
    printf("Erro ao abrir os arquivos.\n");
    exit(1);
  }

  char nomeArquivo[50], linha[100];
  ListaI *aux = listaInclude;

  FILE *arquivoHeader;

  while(aux != NULL) {
    arquivoHeader = fopen(aux->nomeInclude, "r");

    if(arquivoHeader == NULL) {
      printf("Erro ao abrir o arquivo: %s\n", aux->nomeInclude);
      aux = aux->prox;
      continue;
    }

    while(fgets(linha, sizeof(linha), arquivoHeader) != NULL) {
      fputs(linha, output);
    }
    aux = aux->prox;
  }
  fclose(arquivoHeader);
  fclose(output);
}


// Adiciona no final da lista
void addDefine(char *linha) {

  char nomeDefine[30], valorDefine[30];

  sscanf(linha, "#define %s %[^\n]", nomeDefine, valorDefine); //stringScanf
	ListaD *no = malloc(sizeof(ListaD));

  no->nomeDefine = malloc(strlen(nomeDefine) + 1); /* +1 para \0 */
  no->valorDefine = malloc(strlen(valorDefine) + 1);

	strcpy(no->nomeDefine, nomeDefine);
  strcpy(no->valorDefine, valorDefine);

	no->prox = NULL;

	if (listaDefine == NULL) {
    listaDefine = no;

  } else {
    ListaD *aux = listaDefine;

    while(aux->prox != NULL) {
      aux = aux->prox;
    }
    aux->prox = no;
  }
}

int isDefine(char *palavra, char **returnValorDefine) {
  if(listaDefine == NULL) {
    return 0;
  }

  ListaD *aux = listaDefine;
  while(aux != NULL) {
    if(strcmp(aux->nomeDefine, palavra) == 0) {
      *returnValorDefine = malloc(strlen(aux->valorDefine) + 1);
      strcpy(*returnValorDefine, aux->valorDefine);
      return 1;
    }
    aux = aux->prox;
  }
  return 0;
}

void expandirDefines(char *inputName, char *outputName) {
  FILE *input = fopen(inputName, "r");
  FILE *output = fopen(outputName, "a");

  if (input == NULL || output == NULL) {
    printf("Erro ao abrir os arquivos.\n");
    exit(1);
  }

  char *returnValorDefine, c, palavra[100];
  int i = 0;
  
  while((c = getc(input)) != EOF) {
    switch(c) {
      case '\"':
      case '\'':
        escreverStrings(input, output, c);
        break;

      case '#': //Nao escreve defines e includes
        while((c = getc(input)) != '\n' && c != EOF){}
        break;

      case ' ':
      case '\t':
      case '\n':
      case '\r':
      case '\v':
      case '\f':
        break;

      default:

        // Acumular palavras reservadas
        palavra[i] = c;
        palavra[++i] = '\0';

        // Não existe palavra reservada com charEspeciais
        if(!isalnum_(c)) { 
          fputs(palavra, output);
          i = 0;
          continue;
        }

        if(palavraReservada(palavra)) {
          fputs(palavra, output);
          i = 0;

          if(precisaEspaco(input, output, palavra)) {
            putc(' ', output);
          }
        } else if (isDefine(palavra, &returnValorDefine)) {
          fputs(returnValorDefine, output);
          free(returnValorDefine);
          i = 0;
        }
        break;

    }
  }
  fclose(input);
  fclose(output);
}



// argc simboliza a quatidade de argumentos
// arqv simboliza os argumentos
int main(int argc, char *argv[]) {
  int i;

  if(argc < 3) {
    printf("O programa deve ser chamado com os nomes dos arquivos de entrada e saida.\n");
    printf("\nExemplo: \n");
    printf(" > precc Input.c Output.c\n\n");
    exit(1);
  }

  char *input = malloc(strlen(argv[argc-2]) + 1);
  strcpy(input, argv[argc-2]);
  char *output = malloc(strlen(argv[argc-1]) + 1);
  strcpy(output, argv[argc-1]);

  printf("Arquivo de entrada: %s\nArquivo de saida: %s\n", input, output);


  char semComentarios[] = "SemComentarios.c";
  char semEspacos[] = "SemEspacos.c";

  identificarComentarios(input, semComentarios);
  removerEspacos(semComentarios, semEspacos);

  contDefinesIncludes(semEspacos);
  expandirIncludes(output);
  expandirDefines(semEspacos, output);

  return 0;
}
