#pragma once                                                  // Inclusao unica por TU.

#include "components.h"                                       // Tipos Transform/Card/Renderable/CardAnim.
#include "entity.h"                                           // Alias Entity (uint32_t).

#include <unordered_map>                                      // Mapa hash usado para guardar componentes por entidade.
#include <vector>                                             // Vetor de entidades existentes.

// Registry minimalista: cria entidades e guarda componentes em mapas separados.
// Isso evita heranca e deixa claro como ECS organiza dados por tipo.

class Registry
{
    public:
        Entity createEntity()                                 // Cria uma nova entidade e devolve seu ID.
        {
            const Entity entity = nextEntity++;
            entities.push_back(entity);
            return entity;
        }

        void clear()                                          // Reseta o registry inteiro (entre rodadas).
        {
            nextEntity = 1;
            entities.clear();
            transforms.clear();
            cards.clear();
            renderables.clear();
            cardAnims.clear();
        }

        void addTransform(Entity entity, const blackjack3D::Transform& transform)   { transforms[entity] = transform; }
        void addCard(Entity entity, const blackjack3D::Card& card)                  { cards[entity] = card; }
        void addRenderable(Entity entity, const blackjack3D::Renderable& renderable){ renderables[entity] = renderable; }
        void addCardAnim(Entity entity, const blackjack3D::CardAnim& anim)          { cardAnims[entity] = anim; }
        void removeCardAnim(Entity entity)                                          { cardAnims.erase(entity); }

        blackjack3D::Transform* getTransform(Entity entity)
        {
            auto it = transforms.find(entity);
            return it == transforms.end() ? nullptr : &it->second;
        }

        blackjack3D::Card* getCard(Entity entity)
        {
            auto it = cards.find(entity);
            return it == cards.end() ? nullptr : &it->second;
        }

        blackjack3D::Renderable* getRenderable(Entity entity)
        {
            auto it = renderables.find(entity);
            return it == renderables.end() ? nullptr : &it->second;
        }

        blackjack3D::CardAnim* getCardAnim(Entity entity)
        {
            auto it = cardAnims.find(entity);
            return it == cardAnims.end() ? nullptr : &it->second;
        }

        const blackjack3D::Transform* getTransform(Entity entity) const
        {
            auto it = transforms.find(entity);
            return it == transforms.end() ? nullptr : &it->second;
        }

        const blackjack3D::Card* getCard(Entity entity) const
        {
            auto it = cards.find(entity);
            return it == cards.end() ? nullptr : &it->second;
        }

        const blackjack3D::Renderable* getRenderable(Entity entity) const
        {
            auto it = renderables.find(entity);
            return it == renderables.end() ? nullptr : &it->second;
        }

        const blackjack3D::CardAnim* getCardAnim(Entity entity) const
        {
            auto it = cardAnims.find(entity);
            return it == cardAnims.end() ? nullptr : &it->second;
        }

        const std::vector<Entity>& getEntities() const        { return entities; }

    private:

        Entity nextEntity = 1;
        std::vector<Entity> entities;
        std::unordered_map<Entity, blackjack3D::Transform> transforms;
        std::unordered_map<Entity, blackjack3D::Card> cards;
        std::unordered_map<Entity, blackjack3D::Renderable> renderables;
        std::unordered_map<Entity, blackjack3D::CardAnim> cardAnims;
};
