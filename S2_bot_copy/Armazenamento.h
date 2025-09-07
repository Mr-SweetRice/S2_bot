#include <Preferences.h>

class Armazenamento {
private:
    Preferences config;

public:
    // Salvar valor unitário em chave específica
    void salvarValor(const char* chave, float valor) {
        config.begin("armazenamento", false);
        config.putFloat(chave, valor);
        config.end();
    }

    // Ler valor unitário
    float lerValor(const char* chave, float def = 0.0f) {
        config.begin("armazenamento", true);
        float valor = config.getFloat(chave, def);
        config.end();
        return valor;
    }

    // Salvar array de floats
    void salvarArray(const float v[], int n) {
        config.begin("armazenamento", false);
        for (int i = 0; i < n; i++) {
            String chave = "val" + String(i);
            config.putFloat(chave.c_str(), v[i]);
        }
        config.end();
    }

    // Ler array de floats
    void lerArray(float v[], int n) {
        config.begin("armazenamento", true);
        for (int i = 0; i < n; i++) {
            String chave = "val" + String(i);
            v[i] = config.getFloat(chave.c_str(), 0.0f);
        }
        config.end();
    }
};
