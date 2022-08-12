# Lab: shell

### Búsqueda en $PATH

Responder: ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La familia de wrappers pertenecientes a exec tienen la diferencia de que varian sus
parametros segun cada una.Todas las funciones de la familia exec cargan un nuevo programa
en el proceso actual y le proporcionan diversos valores, sin embargo, las diferencias entre 
ellas radican en como se encuentra el programa, de donde proviene el entorno y cuales son los
argumentos.

- Si la wrapper posee "v" significa que posee como parametro una matriz de punteros a cadenas.El primer argumento debe apuntar al nombre del programa asociado con el programa que se va a ejecutar y el ultimo argumento debe ser un puntero nulo.

- Si la wrapper posee "l" significa que posee como parametro una lista de argumentos de
longitud variable compuesta por punteros a cadenas, cuyo ultimo elemento es un puntero
nulo.El primer argumento debe apuntar al nombre del programa que se va a ejecutar.

- Si la wrapper posee "e" significa que posee como parametro un argumento adicional que
proporciona una matriz de punteros a variables de entorno. En el caso contrario el programa hereda el
entorno del proceso actual.

- Si la wrapper posee "p" significa que se va a buscar en el PATH el nombre del programa
que se va a ejecutar.En caso contrario se debe asignar una ruta

La diferencia que posee la syscall execve es que es la unica llamada al sistema de la familia exec, y sus parametros difieren de las wrappers asociados a la familia exec. En este caso los parametros de execve se encuentran dados por:

- Una matriz de punteros a cadenas -> v
- Una matriz de punteros a variables de entorno -> e

Ademas recibe como parametro el nombre de ruta del programa. La syscall execve ejecuta el programa al que se refiere el nombre de la ruta y esto permite que el entorno del proceso que se esta ejecutando actualmente sea reemplazado por el entorno del nuevo programa.


Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Las llamadas a cada uno de los wrappers de la familia exec pueden fallar y en nuestra implementacion se resuelve este problema analizando si el valor del wrapper es equivalente a -1. En este caso lo que haremos es avisar que ocurrio el error a traves de la funcion perror y finalizar el proceso con la syscall exit.

---

### Comandos built-in


Pregunta: ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

El comando cd no puede ser implementado sin necesidad de ser un built-in ya que en este caso , cuando la shell realice un fork, solo el hijo podria cambiar de directorio  y al finalizar la ejecucion, la shell seguiria estando donde se encontraba inicialmente. En el caso de pwd ademas de ser una built-in es un programa ejecutable, y en este caso no habria inconviente al realizar un fork , ya que el proceso hijo compartiria el mismo directorio que el padre. La ventaja de que sea un built-in es que se puede añadir funcionalidad y realizar modificaciones.


---

### Variables de entorno adicionales

Pregunta: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario ejecutar el codigo luego de una llamada a fork, ya que al ser variables temporales deben ser eliminadas tras la finalizacion de un proceso. En el caso de que no utilizemos la syscall fork lo que sucedera es que las variables no seran temporales , ya que su tiempo de vida se mantendra hasta la finalizacion de la shell.

Pregunta: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).
.¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
.Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Si utilizamos la familia de funciones exec que finalizan con la letra e lo que sucedera es que al transformar el proceso no se heredara el entorno del proceso transformado. A partir de ello lo que sucedera es que se eliminaran todas las variables de entorno que teniamos anteriormente, generandose un nuevo entorno. Para que el comportamiento sea el mismo debemos pasar por parametro una lista con todas las variables de entorno que tenemos en el proceso actual y ademas las nuevas variables de entorno temporales que deseamos añadir.

---

### Procesos en segundo plano

Responder: Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

El mecanismo utilizado para implementar el proceso en segundo plano consiste sencillamente en no bloquear la espera de dicho proceso y esto se realiza a traves de la syscall waitpid, la cual al recibir por parametro el flag WNOHANG nos permite continuar con la ejecucion de nuestro programa.

---

### Flujo estándar

Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?

El simbolo > permite redirigir la salida estandar de un proceso a un archivo determinado.En el caso de querer redirigir el error estandar a un descriptor de archivo debemos utilizar el simbolo &, el cual nos permitira distinguir el nombre de los archivos comunes de los descriptores de archivo. Por ejemplo :
Si utilizamos 2>1 lo que haremos es redirigir el error estandar a un archivo cuyo nombre es 1 , por lo que si deseamos redirigir a un descriptor de archivo debemos utilizar el simbolo & , de forma tal que 2>&1 redirigira el contenido del error estandar a la salida estandar.

La instruccion ls -C /home /noexiste lo que hara sera mostrar todos los directorios y archivos que se encuentran en el directorio /home/noexiste , la cual fallara ya que no existe el directorio. La salida estandar sera redirigida al archivo out.txt , y a su vez el error estandar sera redirigido al archivo out.txt. Por lo tanto si ejecutamos el comando cat out.txt lo que sucedera es que se mostrara el contenido del archivo , el cual tendra el error cometido por el comando ls -C /home /noexiste. En el caso inverso , en el que ejecutemos la instruccion ls -C /home /noexiste 2>&1 >out.txt lo que sucedera es que el estandar error sera redirigido a la salida estandar por lo tanto lo que sucedera es que se imprimira el error por pantalla.

---

### Tuberías simples (pipes)

Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.

El exit code devuelve 0 cuando se ejecuta correctamente un pipe.En el caso de que el ultimo comando del pipe cuya salida es la salida estandar lo que va a suceder es que se modificara el valor del exit code.Podemos observar en este ejemplo que al fallar el ultimo comando del pipe lo que sucedera es que el exit code devolvera 2 al imprimirlo.Esta implementacion no esta desarrollada en la shell realizada, ya que no refleja este cambio.

Bash

    $ echo a | ls noexiste
    ls: no se puede acceder a 'noexiste': No existe el archivo o el directorio

    echo $?
    2

Shell 

    $ echo a | ls noexiste
    ls: no se puede acceder a 'noexiste': No existe el archivo o el directorio

    $ echo $?
    0




---

### Pseudo-variables

Pregunta: Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

Otras tres variables magicas son 

\$ : La variable magica $ indica el identificador del proceso actual.En este caso podemos observar que el identificador asociado es el del proceso que esta ejecutando la bash.

    echo $$
    9464

0 : La variable magica 0 indica el nombre del proceso actual.

    echo $0
    bash

! : La variable magica ! indica el identificador del proceso hijo.En este caso podemos observar que el identificador asociado al proceso que ejecuta sleep 50& es 18663.

    sleep 50&
    [1] 18663

    echo $!
    18663



---


