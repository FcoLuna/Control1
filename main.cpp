#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mpi.h>

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
int world_size,world_rank;
int procesadores=0;
MPI_Init(NULL, NULL);
MPI_Comm_size(MPI_COMM_WORLD, &world_size);
MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

string to_string(int n)
{
    ostringstream convert;
    convert<<n;
    return convert.str();
}

void copy_file2( const char* srce_file, const char* dest_file )
{
    std::ifstream srce( srce_file, std::ios::binary ) ;
    std::ofstream dest( dest_file, std::ios::binary ) ;
    dest << srce.rdbuf() ;
}

void copy_file()
{
    for(int i=0;i<world_size;i++)
    {
        string file="Estaciones"+to_string(i)+".txt";
        char* f = new char [file.length()+1];
        strcpy (f, file.c_str());
        copy_file("Estaciones.txt", f);
        if(i>0)MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
}

void remove_file()
{
    for(int i=0;i<world_size;i++)
    {
        string file="Estaciones"+to_string(i)+".txt";
        char* f = new char [file.length()+1];
        strcpy (f, file.c_str());
        remove(f);
    }
}

void buscar_camino_minimo()
{
    for(int i = 0 ; i < procesadores ; i++)
    {
        string cadena;
        string variable="", file=to_string(i)+".txt";
        char* f = new char [file.length()+1];
        strcpy (f, file.c_str());
        ifstream fe(f);
        while(getline (fe,cadena))
        {
            for(int i = 0; i < cadena.size() ; i++)
            {
                if(cadena[i]==';')
                {
                    stringstream a(s);
                    int x;
                    a >> x;
                    if(x<num_estaciones_min)
                    {
                        num_estaciones_min=x;
                        variable="";
                        for(int j = i+1 ; j<cadena.size();j++)
                            variable+=cadena[j];
                        camino_minimo=variable;
                        variable="";
                    }
                    break;
                }
                else
                    variable+=cadena[i];
            }
            break;
        }
        fe.close();
    }
}

void reducir_camino_minimo_file()
{
    string cadena;
    string variable="", file=to_string(world_rank)+".txt";
    char* f = new char [file.length()+1];
    strcpy (f, file.c_str());
    ifstream fe(f);
    while(getline (fe,cadena))
    {
        for(int i = 0; i < cadena.size() ; i++)
        {
            if(cadena[i]==';')
            {
                stringstream a(s);
                int x;
                a >> x;
                if(x<num_estaciones_min)
                {
                    num_estaciones_min=x;
                    variable="";
                    for(int j = i+1 ; j<cadena.size();j++)
                        variable+=cadena[j];
                    camino_minimo=variable;
                    variable="";
                }
                break;
            }
            else
                variable+=cadena[i];
        }
    }
    fe.close();
    ofstream fs(file.c_str(), ios::trunc);
    fs<<to_string(num_estaciones_min)<<";"<<camino_minimo<<"\n";
}

// estructura Lista
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

//Funcion Encargada de abrir el archivo con las estaciones y los convierte en una lista
void file_to_tree ()
{
    string cadena;
    string file="Estaciones"+to_string(world_rank)+".csv";
    char* f = new char [file.length()+1];
    strcpy (f, file.c_str());
    ifstream fe(f);
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
    if(world_rank)
    {
        int dummy=1;
        MPI_Send(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
}

void numero_caminos(Lista nodo)
{
    return ((nodo->p_1)?1:0) + ((nodo->p_2)?1:0) + ((nodo->p_3)?1:0) + ((nodo->p_4)?1:0);
}

//Recorre la lista
void recorrer_arbol2(Lista inicio, int origen, string estacion_inicial, string estacion_final, int flag_inicial, int num_recorrido, string camino, int procesadores_usados)
{
    if(inicio != NULL && num_recorrido < num_estaciones)
    {
        if(inicio->nombre==estacion_final)
        {
            camino=camino+" - "+inicio->nombre;
            string file=to_string(world_rank)+".txt";
            char* f = new char [file.length()+1];
            strcpy (f, file.c_str());
            ofstream fs(file.c_str(), ios::app);
            fs<<to_string(num_recorrido)<<";"<<camino<<"\n";
            fs.close();
        }
        else
        {
                if(inicio->ingresos<ingresos_maximos && (inicio->nombre!=estacion_inicial || flag_inicial))
                {
                    int num_camino=numero_caminos(inicio);
                    inicio->ingresos+=1;
                    if(!flag_inicial) camino=camino+" - "+inicio->nombre;
                    if(num_camino==1 || world_rank || (procesadores_usados + num_camino-1)>= world_size)
                    {
                        if(origen!=2 && inicio->p_1)recorrer_arbol2(inicio->p_1,1,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                        if(origen!=1 && inicio->p_2)recorrer_arbol2(inicio->p_2,2,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                        if(origen!=4 && inicio->p_3)recorrer_arbol2(inicio->p_3,3,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                        if(origen!=3 && inicio->p_4)recorrer_arbol2(inicio->p_4,4,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                    }
                    else
                    {
                        if(!world_rank)
                        {
                            int a_crear = procesadores_usados + num_camino-1;
                            bool accesos[3], flag=true;
                            if(inicio->p_1)?accesos[0]=true:accesos[0]=false;
                            if(inicio->p_2)?accesos[1]=true:accesos[0]=false;
                            if(inicio->p_3)?accesos[2]=true:accesos[0]=false;
                            for(int i = procesadores_usados ; i < a_crear ; i++)
                            {
                                string file="to_"+to_string(i)+".txt";
                                char* f = new char [file.length()+1];
                                strcpy (f, file.c_str());
                                ofstream fs(f);

                                string ini, ori,num_rec=to_string(num_recorrido+1);
                                if(origen!=2 && accesos[0] && flag){ini=inicio->p_1->codigo;accesos[0]=false;flag=false;ori="1";}
                                if(origen!=1 && accesos[1] && flag){ini=inicio->p_2->codigo;accesos[1]=false;flag=false;ori="2";}
                                if(origen!=4 && accesos[2] && flag){ini=inicio->p_3->codigo;accesos[2]=false;flag=false;ori="3";}
                                flag=true;
                                fs<<ini<<";"<<ori<<";"<<estacion_inicial<<";"<<estacion_final<<";"<<num_rec<<";"<<camino;
                                fs.close();
                                MPI_Send(1, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                            }
                            procesadores_usados=a_crear;
                            procesadores=procesadores_usados;
                            if(origen!=1 && accesos[1]){
                                recorrer_arbol2(inicio->p_2,2,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                            }
                            else
                                if(origen!=4 && accesos[2]){
                                    recorrer_arbol2(inicio->p_3,3,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                                }
                                else{
                                    recorrer_arbol2(inicio->p_4,4,estacion_inicial,estacion_final,0,num_recorrido+1,camino,0);
                                }
                        }
                    }
                }
        }
    }
}

// Funcion para validar estacion de inicio y estacion de termino
void recorrer_arbol(string estacion_inicial, string estacion_final )
{
    Lista inicio=NULL;
    buscar_nodo(inicio,estacion_final);
    if(inicio==NULL)
        cout<<"Estacion final invalida"<<endl;
    else
    {
        estacion_final=inicio->nombre;
        buscar_nodo(inicio,estacion_inicial);
        if(inicio==NULL)
            cout<<"Estacion inicial invalida"<<endl;
        else
            {
                estacion_inicial=inicio->nombre;
                recorrer_arbol2(inicio,0,inicio->nombre,estacion_final,1,0,inicio->nombre,1);
            }
    }
}

//Muestra el resutado del camino minimo

void iniciar(string estacion_inicial, string estacion_final)
{
    if(!world_rank)
        copy_files();
    else
        {
            int dummy;
            MPI_Recv(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
    file_to_tree();
    if(!world_rank)
    {
        for(int i = 1, dummy ; i < world_size ; i++)
            MPI_Recv(&dummy, 1, MPI_INT, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        remove_file();
    }
    num_estaciones_min=num_estaciones;
    ingresos_maximos=num_estaciones;
    if(!world_rank)
    {
        recorrer_arbol(estacion_inicial,estacion_final);
        for(int i=procesadores; i<world_size;i++)
            MPI_Send(0, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        reducir_camino_minimo_file();
        buscar_camino_minimo();
        cout<<"Debe recorrer "<<num_estaciones_min<<" para llegar a su destino\n";
        cout<<camino_minimo<<endl<<endl;
    }
    else
        {
            int dummy;
            MPI_Recv(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            if(dummy)
            {
                Lista l=NULL;
                int ori,num_rec;
                string ini, est_ini, est_fin,camino,variable="", line, file="to_"+to_string(world_rank)+".txt";
                char* f = new char [file.length()+1];
                strcpy (f, file.c_str());
                ifstream file_aux (f);
                getline (file_aux,line);
                for(int i=0, flag=0 ; i < line.size() ; i++)
                {
                    if(line[i]==';')
                    {
                        switch(flag){
                            case 0:
                                ini=variable;
                                break;
                            case 1:
                                stringstream a(variable);
                                a>>ori
                                break;
                            case 2:
                                est_ini=variable;
                                break;
                            case 3:
                                est_fin=variable;
                                break;
                            case 4:
                                stringstream a(variable);
                                a>>num_rec;
                                break;
                        }
                        flag++;
                        variable="";
                    }
                    else
                        variable+=line[i];
                }
                camino=variable;
                buscar_nodo(l, ini);
                recorrer_arbol2(l,ori,est_ini,est_fin,0,num_rec,camino,1);
                reducir_camino_minimo_file();
            }
        }
}

int main(int argc, char* argv[])
{
    if( argc < 2 )
        if(!world_rank) cout << "Debe ingresar al menos 1 argumento" << endl;
    else
        if(string(argv[1])  == "-f")
            if(argc == 4)
                iniciar(string(argv[2]),string(argv[3]));
            else
                if(!world_rank) cout << "Debe ingresar correctamente las estaciones" << endl;
        else
            if(string(argv[1])  == "-v" && !world_rank)
                cout << "Integrantes: \n\t - Francisco Luna\n\t - Ignacio Araya\n - Erwin Alvez\n" << endl;
    MPI_Finalize();
    return 0;
}
