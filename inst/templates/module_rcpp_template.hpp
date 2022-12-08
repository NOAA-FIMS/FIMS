/****************************************************************
 * {{{ module_type }}} Rcpp interface                                   *
 ***************************************************************/
class {{{ module_type }}}_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, {{{ module_type }}}_interface_base*> {{{ module_type }}}_objects;

    {{{ module_type }}}_interface_base() {

    }

    virtual ~{{{ module_type }}}_interface_base() {
    }

};

uint32_t {{{ module_type }}}_interface_base::id_g = 1;
std::map<uint32_t, {{{ module_type }}}_interface_base* > {{{ module_type }}}_interface_base::{{{ module_type }}}_objects;

/**
 * @brief Interface class for {{{module_name}}} {{{ module_type }}}. 
 */
class {{{ module_name }}} : public {{{ module_type }}}_interface_base {
public:
    uint32_t id;
    {{{ parameter_initialize }}}

    {{{ module_name }}}() {
        this->id = {{{ module_type }}}_interface_base::id_g++;
        {{{ module_type }}}_interface_base::{{{ module_type }}}_objects[this->id] = this;
        fims_rcpp_interface_base::fims_interface_objects.push_back(this);
    }

    virtual ~{{{ module_name }}}() {
    }