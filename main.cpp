#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef struct Nodo{
    string linea;
    string codigo;
    string nombre;
    int ingresos;
    Nodo* p_1;
    Nodo* p_2;
    Nodo* p_3;
    Nodo* p_4;
};
typedef Nodo* Lista;

vector <Lista> malla;
string camino_minimo="";
int num_estaciones=0;
int num_estaciones_min;
int ingresos_maximos;

void buscar_nodo(Lista &in_nodo, string codigo)
{
    for(int i = 0 ; i < malla.size() ; i++)
    {
        Lista aux = malla[i];
        while(aux!=NULL)
        {
            if(aux->codigo == codigo)
            {
                in_nodo=aux;
                return;
            }
            else
                aux=aux->p_1;
        }
    }
    in_nodo=NULL;
}

/**
    El formato de lectura de las estaciones corresponde a:
        [Nombre línea];[codigo-1]-[nombre estacion-1];[codigo-2]-[nombre estacion-2];...;[codigo-n]-[nombre estacion-n];
    Siendo cada línea del archivo una línea hasta encontrarse un "--" que significa que empiezan las combinaciones
*/

void file_to_tree ()
{
    string cadena;
    ifstream fe("Estaciones.txt");
    int combinaciones=0;
    while(getline (fe,cadena)) {
        if(cadena=="--")
        {
            combinaciones=1;
            continue;
        }
        if(!combinaciones)
        {
            Lista linea = NULL;
            string nombre_linea="";
            string estacion = "";
            string codigo = "";
            int estado=1;
            for(int i = 0 ; i < cadena.length() ; i++)
            {
                if(estado && cadena[i]!=';' && cadena[i]!='-')
                    codigo+=cadena[i];
                else
                    if(cadena[i]!=';')
                    {
                        estado=0;
                        if(cadena[i]!='-')
                            estacion+=cadena[i];
                    }
                    else
                    {
                            if(estacion.length()==0)
                                nombre_linea=codigo;
                            else
                                {
                                    num_estaciones++;
                                    Lista p;
                                    p = new(Nodo);
                                    if(linea==NULL)
                                    {
                                        p->p_1=NULL;
                                    }
                                    else
                                    {
                                        linea->p_2=p;
                                        p->p_1=linea;
                                    }
                                    p->p_2=NULL;
                                    p->p_3=NULL;
                                    p->p_4=NULL;
                                    p->codigo=codigo;
                                    p->linea=nombre_linea;
                                    p->nombre=estacion;
                                    p->ingresos=0;
                                    linea=p;
                                }
                            estacion="";
                            codigo="";
                            estado=1;
                    }
            }
            malla.push_back(linea);
        }
        else
        {
            string estacion_1 = "";
            string estacion_2 = "";
            int estado=0;
            for(int i = 0 ; i < cadena.length() ; i++)
            {
                if(cadena[i]=='-')
                {
                    estado=1;
                    continue;
                }
                if(estado)
                    estacion_2+=cadena[i];
                else
                    estacion_1+=cadena[i];
            }
            Lista aux1, aux2;
            buscar_nodo(aux1,estacion_1);
            buscar_nodo(aux2,estacion_2);
            if(aux1!=NULL && aux2!=NULL)
            {
                aux1->p_3=aux2->p_1;
                aux1->p_4=aux2->p_2;
                aux2->p_3=aux1->p_1;
                aux2->p_4=aux1->p_2;
            }
        }
    }
    fe.close();
}

void recorrer_arbol2(Lista inicio, int origen, string estacion_inicial, string estacion_final, int flag_inicial, int num_recorrido, string camino)
{
    if(inicio != NULL && num_recorrido < num_estaciones)
    {
        if(inicio->codigo==estacion_final)
        {
            if(num_recorrido < num_estaciones_min)
            {
                camino=camino+" - "+inicio->nombre;
                camino_minimo=camino;
                num_estaciones_min=num_recorrido;
            }
        }
        else
        {
            if(inicio->ingresos<ingresos_maximos && (inicio->codigo!=estacion_inicial || flag_inicial))
            {
                inicio->ingresos+=1;
                if(!flag_inicial) camino=camino+" - "+inicio->nombre;
                if(origen!=2) recorrer_arbol2(inicio->p_1,1,estacion_inicial,estacion_final,0,num_recorrido+1,camino);
                if(origen!=1)recorrer_arbol2(inicio->p_2,2,estacion_inicial,estacion_final,0,num_recorrido+1,camino);
                if(origen!=4)recorrer_arbol2(inicio->p_3,3,estacion_inicial,estacion_final,0,num_recorrido+1,camino);
                if(origen!=3)recorrer_arbol2(inicio->p_4,4,estacion_inicial,estacion_final,0,num_recorrido+1,camino);
            }
        }
    }
}

void recorrer_arbol(string estacion_inicial, string estacion_final )
{
    Lista inicio=NULL;
    buscar_nodo(inicio,estacion_final);
    if(inicio==NULL)
        cout<<"Estacion final invalida"<<endl;
    else
    {
        buscar_nodo(inicio,estacion_inicial);
        if(inicio==NULL)
            cout<<"Estacion inicial invalida"<<endl;
        else
            {
                recorrer_arbol2(inicio,0,inicio->codigo,estacion_final,1,0,inicio->nombre);
            }
    }
}

void iniciar(string estacion_inicial, string estacion_final)
{
    file_to_tree();
    num_estaciones_min=num_estaciones;
    ingresos_maximos=num_estaciones;
    recorrer_arbol(estacion_inicial,estacion_final);
    cout<<"Debe recorrer "<<num_estaciones_min<<" para llegar a su destino\n";
    cout<<camino_minimo<<endl<<endl;
}

int main(int argc, char* argv[])
{
    if( argc < 2 )
        cout << "Debe ingresar al menos 1 argumento" << endl;
    else
        if(argv[1]  == "-f")
            if(argc == 4)
                iniciar(argv[2],argv[3]);
            else
                cout << "Debe ingresar correctamente las estaciones" << endl;
        else
            if(argv[1]  == "-v")
                cout << "Integrantes: \n\t - Francisco Luna\n\t - Ignacio Araya\n" << endl;
    return 0;
}
