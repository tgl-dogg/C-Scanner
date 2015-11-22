Autor: Vinicius de Carvalho.

Trabalho desenvolvido para a disciplina de Paradigmas de Linguagens de Programação, sob a orientação do professor Eduardo Takeo Ueda.

Instruções:
1. Execute o Makefile do projeto.
- Acesse a pasta do projeto via terminal e execute o comando "make"

2. Execute o arquivo principal.
- Acesse a pasta do projeto via terminal e execute o comando "./scanner INPUT OUTPUT"
- INPUT deverá ser o arquivo de entrada a ser testado. Este arquivo precisa existir.
- OUPUT deverá ser o arquivo de saída para conter o reusltado. Sua existência não é obrigatória, mas caso já exista, será sobreescrito.
Exemplo: ./scanner source.dogg source.txt

Observações:
I. Podem haver problemas com palavras com mais de 65535 (2^16-1) dígitos, não havia limite na especificação então eu defini um.
II. Os arquivos .dogg são casos de teste e os arquivos .txt são a saída dada quando fornecidos estes casos de teste.
III. Erros são indicados no arquivo de saída mas não pausam o programa.
IV. Alguns erros podem ser truncados pelo limite definido no item I, neste caso, o scanner tentará analisar normalmente o restante do arquivo, podendo se deparar com erros novamente.