# Project Genesis

**Um projeto de aprendizado para construir um kernel de sistema operacional de 64 bits, do zero.**

Project Genesis é um kernel x86_64 educacional escrito em C e Assembly. O objetivo inicial é deliberadamente pequeno: compilar um kernel bootável, carregá-lo com o GRUB e imprimir uma mensagem numa máquina virtual QEMU. A partir daí, o projeto cresce em direção a interrupções, gerenciamento de memória, drivers de dispositivo, multitarefa e um shell básico.

> **Situação:** marcos 1 e 2 — o kernel compila e linka; o boot no QEMU ainda depende das ferramentas do GRUB (veja [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md))  
> **Alvo:** x86_64 (64 bits)  
> **Linguagens:** C e Assembly x86_64  
> **Protocolo de boot:** Multiboot2, com header Multiboot1 como alternativa  
> **Ambiente de teste:** QEMU

## Objetivos do projeto

- Entender o processo de boot e a fronteira entre firmware, bootloader e kernel.
- Construir um kernel pequeno de forma incremental, mantendo cada marco testável.
- Aprender arquitetura de CPU, gerenciamento de memória, interrupções e I/O de dispositivos.
- Manter documentação clara e instruções de build reprodutíveis.

## Roadmap

| Marco | Objetivo | Situação |
|---|---|---|
| 0. Toolchain | Preparar compilador, assembler, linker, ferramentas do GRUB e QEMU | Parcial — LLVM, make e QEMU instalados; faltam as ferramentas do GRUB na máquina Windows |
| 1. Boot | Carregar um kernel mínimo pelo GRUB | Código completo — a imagem compila e linka, o boot depende das ferramentas do GRUB |
| 2. Saída | Imprimir texto num console serial | Código completo — driver escrito, a saída depende do boot |
| 3. Configuração da CPU | Estabelecer as estruturas GDT e IDT | Não iniciado |
| 4. Interrupções | Tratar exceções e interrupções de hardware | Não iniciado |
| 5. Memória | Adicionar gerenciamento de memória física e virtual | Não iniciado |
| 6. Dispositivos | Adicionar suporte a timer e teclado | Não iniciado |
| 7. Escalonamento | Introduzir threads de kernel e escalonamento cooperativo | Não iniciado |
| 8. Armazenamento | Explorar dispositivos de bloco e um sistema de arquivos simples | Não iniciado |
| 9. Modo usuário | Estabelecer a fronteira usuário/kernel e chamadas de sistema | Não iniciado |
| 10. Shell | Construir uma interface de linha de comando básica | Não iniciado |

## Estrutura do repositório

```text
Project-Genesis/
├── boot/                     # Protocolo de boot, linker script e código de entrada
├── docs/                     # Notas de projeto e material de estudo
├── include/                  # Headers do kernel (espelham a estrutura de kernel/)
│   └── arch/x86_64/          #   io.h, serial.h, boot_info.h
├── kernel/                   # Implementação do kernel
│   ├── main.c                #   ponto de entrada: o menor caminho de inicialização possível
│   ├── arch/                 #   código específico de CPU e plataforma
│   │   └── x86_64/           #     console serial, decodificação das informações de boot
│   ├── drivers/              #   drivers de dispositivo
│   ├── mm/                   #   gerenciamento de memória física e virtual
│   ├── sched/                #   threads de kernel e escalonamento
│   ├── fs/                   #   sistemas de arquivos
│   └── lib/                  #   utilitários freestanding compartilhados pelo kernel
├── scripts/                  # Scripts auxiliares de build e execução
├── tests/                    # Planos de teste e futuros testes automatizados
├── .gitignore
├── CONTRIBUTING.md
├── LICENSE
├── Makefile
└── README.md
```

## Princípios de desenvolvimento

1. **Passos pequenos:** uma funcionalidade compreensível por mudança.
2. **Testar no emulador:** não experimente no disco de boot do seu computador principal.
3. **Documentar as decisões:** registre por que uma abordagem foi escolhida.
4. **Manter o kernel freestanding:** não assuma que exista uma biblioteca C do sistema hospedeiro.
5. **Verificar cada marco:** uma funcionalidade só está pronta quando o seu teste está documentado.

## Compilando

O kernel é freestanding e, por padrão, compilado com LLVM (`clang` + `ld.lld`); um cross-compiler GCC também funciona, passando `CC=`/`LD=`. O QEMU dá boot na ISO do GRUB, não no ELF puro — o carregador Multiboot embutido no QEMU rejeita imagens de 64 bits.

```sh
make check   # versões das ferramentas e o que está faltando
make         # build/kernel.elf
make iso     # genesis.iso (precisa de grub-mkrescue + xorriso)
make run     # boot da ISO no QEMU, console serial neste terminal
make test    # boot sem interface gráfica, verifica a saída serial
```

Saída esperada no console serial (COM1, 115200 8N1) — o kernel compila, mas essa saída ainda não foi capturada porque não há ferramentas do GRUB instaladas na máquina de desenvolvimento:

```text
Project Genesis
===============
boot magic : 0x36d76289
protocol   : multiboot2
bootloader : GRUB 2.12
cpu mode   : 64-bit long mode
result     : OK - kernel entry reached and serial output works
```

Veja [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md) para as versões testadas e a preparação do ambiente, e [docs/ROADMAP.md](docs/ROADMAP.md) para o que está pronto e o que vem agora.

## Como foi feito

Esta seção registra como a fatia de boot e kernel foi implementada e por que cada decisão foi tomada, para que a próxima pessoa continue sem precisar redescobrir tudo.

### 1. O contrato de boot

`boot/boot.s` coloca um header **Multiboot2** nos primeiros bytes da imagem (ele precisa estar nos primeiros 32 KiB do arquivo), seguido de um header **Multiboot1**. O GRUB usa Multiboot2; o header mais antigo mantém a imagem carregável por loaders que só implementam o primeiro protocolo. `kmain` inspeciona o valor mágico que recebe (`0x36D76289` para Multiboot2, `0x2BADB002` para Multiboot1) e informa qual deles entregou o controle, então a alternativa fica visível em vez de silenciosa.

Os dois headers ficam em `KEEP(*(.multiboot))` no `boot/linker.ld`, ordenados no início da imagem, porque o linker descartaria ou reordenaria uma seção não referenciada e o loader nunca encontraria o valor mágico.

### 2. De 32 bits para long mode

O GRUB entrega o controle em **modo protegido de 32 bits** com paginação desligada, então o primeiro código que roda é Assembly:

1. `cli` + `cld` e, em seguida, salvar o valor mágico e o ponteiro de informações de boot antes de reutilizar qualquer registrador (eles chegam em `eax` e `ebx`).
2. Montar um mapeamento de identidade de três níveis: um PDPT e um PD com 512 entradas de **páginas de 2 MiB**, cobrindo o primeiro 1 GiB — suficiente para um kernel carregado em 1 MiB, e mais barato que páginas de 4 KiB porque não precisa de tabela de segundo nível.
3. Habilitar `CR4.PAE`, carregar `CR3` com o PML4, setar `EFER.LME` e então habilitar `CR0.PG` e `CR0.WP` — este é o passo que entra no modo de compatibilidade do long mode.
4. Carregar uma GDT flat de 64 bits e fazer o *far jump* para o seletor de código de 64 bits, que é o que de fato coloca a CPU em modo 64 bits.
5. Zerar o `.bss`, montar uma pilha de 16 KiB e chamar `kmain`.

As tabelas de página vivem no `.bss` e são zeradas explicitamente pelo stub, sem assumir que já estejam zeradas: o loader não é obrigado a zerar uma seção `NOBITS`.

### 3. O lado do kernel

`kernel/main.c` é C freestanding, sem dependência de biblioteca hospedeira, e se mantém o menor possível: inicializa o console serial, pede ao subsistema de boot que decodifique a entrega e reporta o resultado. Todo o detalhe de protocolo vive em `kernel/arch/x86_64/boot_info.c`, que percorre a lista de tags do Multiboot2 (ou a estrutura do Multiboot1) para descobrir o nome do bootloader e os tamanhos de memória. Um valor mágico errado resulta em protocolo `unknown` em vez de uma estrutura inválida, então o kernel nunca confia cegamente no loader.

`kernel/arch/x86_64/serial.c` controla uma UART compatível com 16550 em **COM1 (0x3F8)**: desliga interrupções, ajusta o divisor para 115200 baud, seleciona 8 bits de dados / sem paridade / 1 stop bit, habilita a FIFO e então faz polling do bit de transmissor vazio antes de cada byte. Ainda não há interrupções, o que mantém este marco testável sem uma IDT.

### 4. Decisões de build

- **LLVM em vez de um cross-compiler GCC.** `clang --target=x86_64-unknown-none-elf` alcança o mesmo alvo bare-metal com `ld.lld` e um linker script próprio, sem construir uma toolchain cruzada. `CC=`/`LD=` continuam aceitando `x86_64-elf-gcc`/`x86_64-elf-ld`. Detalhes em [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md).
- **Flags freestanding que importam:** `-ffreestanding -fno-builtin`, `-fno-pic -fno-pie` (endereço de carga fixo), `-mno-red-zone` (handlers de interrupção não podem usá-la), `-mgeneral-regs-only` (nenhum estado SSE no kernel por enquanto) e `-fno-asynchronous-unwind-tables` (sem `.eh_frame`).
- **`ASFLAGS` separado.** `-O2`, `-std=gnu11` e companhia são exclusivos de C; o clang os rejeita como argumentos não usados ao montar um arquivo `.s`, e com `-Werror` isso quebra o build. O Assembly recebe apenas o alvo e `-Iinclude`.
- **`ifeq ($(origin CC),default)`** no Makefile: o GNU make predefine `CC=cc`, então `CC ?= clang` mantém `cc` silenciosamente. O condicional é o que realmente seleciona o alvo cruzado.
- **`SHELL := sh` no Windows**, porque o shell padrão das receitas é o `cmd.exe`, que não tem `mkdir -p` nem `rm`.
- **Descoberta de fontes recursiva** (`$(wildcard kernel/*.c kernel/*/*.c kernel/*/*/*.c)`): mover um subsistema para seu próprio diretório não exige editar o Makefile. Foi usado `wildcard` em vez de `find` para não depender de shell.

### 5. Como foi verificado

- `make clean && make` → exit 0, produz `build/kernel.elf` (ELF64, `EM_X86_64`, entrada `0x101000`).
- Conferência dos headers no arquivo linkado: valor mágico do Multiboot2 no offset `0x1000`, alinhado a 8 bytes, checksum do header nulo; Multiboot1 em `0x1018`, alinhado a 4 bytes, checksum nulo.
- `make check` → exit 0, reporta versões das ferramentas e o que falta.
- `sh -n` em todos os scripts → nenhum erro de sintaxe.

**Ainda não verificado:** um boot de verdade. O carregador Multiboot embutido no QEMU rejeita imagens de 64 bits (`Cannot load x86-64 image, give a 32bit one`), então o kernel precisa ser bootado a partir da ISO do GRUB, e `grub-mkrescue`/`xorriso` não estão disponíveis na máquina Windows atual. O `make test` falha com uma mensagem explícita até que eles sejam instalados (WSL ou uma máquina Linux). Essa é a primeira tarefa da próxima fatia, antes que qualquer trabalho no kernel possa ser chamado de "funcionando".

## Segurança

Use o QEMU ou outra máquina virtual descartável para experimentos. Não grave imagens de boot experimentais em um disco real nem altere o bootloader do seu computador enquanto estiver aprendendo.

## Licença

Veja [LICENSE](LICENSE).
