/**
    @file
    delta - computes differences
    andrew malozemoff - andrew@luckycatmusic.com

    @ingroup    custom
*/

#include "ext.h"
#include "ext_obex.h"

typedef struct _delta
{
    t_object    ob;
    t_atom      last;
    void        *m_outlet1;
} t_delta;

// prototypes
void *delta_new(t_symbol *s, long argc, t_atom *argv);
void delta_free(t_delta *x);
void delta_assist(t_delta *x, void *b, long m, long a, char *s);

void delta_int(t_delta *x, long n);
void delta_float(t_delta *x, double f);
void delta_list(t_delta *x, t_symbol *s, long ac, t_atom *av);
void delta_msg(t_delta *x, t_symbol *s, long ac, t_atom *av);
void delta_bang(t_delta *x);

// global class pointer
void *delta_class;

// main
void ext_main(void *r)
{
    t_class *c;

    c = class_new("delta", (method)delta_new, (method)delta_free, (long)sizeof(t_delta), 0L, A_GIMME, 0);

    class_addmethod(c, (method)delta_assist,        "assist",   A_CANT,     0);
    class_addmethod(c, (method)delta_int,           "int",      A_LONG,     0);
    class_addmethod(c, (method)delta_float,         "float",    A_FLOAT,    0);
    class_addmethod(c, (method)delta_list,          "list",     A_GIMME,    0);
    class_addmethod(c, (method)delta_msg,           "anything", A_GIMME,    0);
    class_addmethod(c, (method)delta_bang,          "bang",                 0);


    class_register(CLASS_BOX, c);
    delta_class = c;
}

// methods
void *delta_new(t_symbol *s, long argc, t_atom *argv)
{
    t_delta *x = NULL;

    if ((x = (t_delta *)object_alloc(delta_class))) {
        x->m_outlet1 = outlet_new(x, NULL);

        if(argc) {
            if(argv->a_type == A_FLOAT) {
                atom_setfloat(&x->last, atom_getfloat(argv));
            } else {
                atom_setlong(&x->last, atom_getlong(argv));
            }
        } else {
            atom_setlong(&x->last, 0);
        }
    }

    return (x);
}

void delta_assist(t_delta *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "int, float, or list of numbers");
    }
    else {  // outlet
        sprintf(s, "change from last input, or list of delta values");
    }
}

void delta_free(t_delta *x)
{
    ;
}

void delta_int(t_delta *x, long n)
{
    long l = atom_getlong(&x->last);
    atom_setlong(&x->last, n);
    outlet_int(x->m_outlet1, n - l);
}

void delta_float(t_delta *x, double f)
{
    double l = atom_getfloat(&x->last);
    atom_setfloat(&x->last, f);
    outlet_float(x->m_outlet1, f - l);
}

void delta_list(t_delta *x, t_symbol *s, long ac, t_atom *av) {
    t_atom *outlist = (t_atom *)sysmem_newptr(sizeof(t_atom) *ac);
    long i;
    t_atom *ap, *oe;

    for(i = 0, ap = av, oe = outlist; i < ac; i++, ap++, oe++) {
        if(x->last.a_type == A_LONG && ap->a_type == A_LONG) {
            atom_setlong(oe, atom_getlong(ap) - atom_getlong(&x->last));
            atom_setlong(&x->last, atom_getlong(ap));
        } else {
            atom_setfloat(oe, atom_getfloat(ap) - atom_getfloat(&x->last));
            atom_setfloat(&x->last, atom_getfloat(ap));
        }
    }

    outlet_list(x->m_outlet1, NULL, ac, outlist);

    sysmem_freeptr(outlist);
}

// method of miscellaneous messages
void delta_msg(t_delta *x, t_symbol *s, long ac, t_atom *av)
{
    t_atom *a;

    if(s == gensym("set") && ac) {
        a = av + ac - 1; // use last element in list
        if(a->a_type == A_FLOAT) {
            atom_setfloat(&x->last, atom_getfloat(a));
        } else if(a->a_type == A_LONG) {
            atom_setlong(&x->last, atom_getlong(a));
        } else {
            object_error((t_object *)x, "improper argument to message 'set'");
        }
    } else if(s == gensym("clear")) {
        atom_setlong(&x->last, (long)0);
    }
}

void delta_bang(t_delta *x)
{
    if(x->last.a_type == A_FLOAT) {
        delta_float(x, atom_getfloat(&x->last));
    } else {
        delta_int(x, atom_getlong(&x->last));
    }
}
