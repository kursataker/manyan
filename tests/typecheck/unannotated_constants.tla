---- MODULE unannotated_constants ----
\* Without annotations on constants alone, Apalache names the last
\* constant in the order of SANY's table of the module's names.
CONSTANT Mid, ClientCfg, SDPSvrCfg, SvrCfg, Alpha, Zeta
VARIABLE
    \* @type: Int;
    x
Init == x = 0
====
