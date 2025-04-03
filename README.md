# Sistema de Monitoramento Industrial
Sistema de segurança com IoT para desligamento de máquinas superaquecidas, controlado via web com interface mobile responsiva. Projeto em grupo feito em <ins>C e HTML</ins> utilizando o microcontrolador <ins>ESP32</ins>.

## Como ele funciona?
Há três componentes principais: O sensor de temperatura e o módulo relê, ambos conectados ao ESP32, e a interface web.
O módulo relê é responsável por fornecer ou cortar a energia das máquinas. Para controlar o módulo relê, utiliza-se uma interface web, onde é possível configurar dois modos de desligamento:
- **Desligamento via Timer** ― o módulo relê forçadamente desliga após um certo período de tempo estabelecido pelo usuário;
- **Desligamento via Temperatura** ― o módulo relê forçadamente desliga após o sensor de temperatura relatar uma temperatura acima do limite estabelecido pelo usuário.

Também é possível habilitar ambos ao mesmo tempo para maior segurança.

Tanto o ESP32 quanto a máquina deverão estar conectados à mesma rede Wi-Fi. Não tivemos tempo para adicionar a funcionalidade de configurar uma rede Wi-Fi para o ESP32 de maneira fácil, então ele deve ser configurado no código bruto.
