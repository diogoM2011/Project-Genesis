# Código-fonte do kernel

O ponto de entrada e o caminho de inicialização ficam o menores possível. Um subsistema ganha seu próprio diretório assim que cresce além de algumas linhas, então este arquivo é o mapa de onde cada coisa mora.

```text
kernel/
├── main.c              ponto de entrada (kmain): liga os subsistemas e reporta a entrega do boot
├── arch/
│   └── x86_64/         código específico de CPU e plataforma
├── drivers/            drivers de dispositivo (timer, teclado, armazenamento, ...)
├── mm/                 gerenciamento de memória física e virtual
├── sched/              threads de kernel e escalonamento
├── fs/                 sistemas de arquivos
└── lib/                utilitários freestanding compartilhados pelo kernel (formatação, strings, ...)
```

Os headers espelham esta estrutura em `include/`, então o caminho de um header sempre diz a qual implementação ele pertence: `include/arch/x86_64/serial.h` combina com `kernel/arch/x86_64/serial.c`.

## Conteúdo atual

| Caminho | O que tem |
|---|---|
| `main.c` | `kmain`: inicializa o console serial, decodifica a entrega do boot, reporta o resultado e para a CPU |
| `arch/x86_64/serial.c` | driver da UART 16550 em COM1 (115200 8N1, polling) |
| `arch/x86_64/boot_info.c` | decodificação de Multiboot1/Multiboot2: nome do bootloader e tamanhos de memória |
| `drivers/`, `mm/`, `sched/`, `fs/`, `lib/` | vazios, mantidos com um `.gitkeep` até chegar o primeiro arquivo real |

## Regras

1. **`main.c` fica pequeno.** Ele pode inicializar subsistemas e imprimir diagnósticos. Qualquer lógica que tenha nome próprio pertence a um subsistema.
2. **Código específico de arquitetura vai em `arch/x86_64/`.** Port I/O, MSRs, frames de interrupção, formatos de tabela de página e protocolos de boot não são portáveis; não os deixe vazar para `main.c` ou `lib/`.
3. **`lib/` tem que continuar freestanding.** Nenhuma chamada de biblioteca hospedeira, nenhuma alocação antes de o alocador existir.
4. **Um subsistema por mudança de diretório.** Um diretório novo aparece na mesma mudança que traz seu primeiro arquivo-fonte de verdade, e o `.gitkeep` sai nesse momento.
5. **O stub do bootloader não fica aqui.** O código de entrada de 32 bits, as tabelas de página, a GDT e o linker script vivem em `boot/`, porque rodam antes de existir C.
