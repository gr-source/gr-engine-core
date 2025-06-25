#pragma once

#include "default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/gameObject.hpp"

class FilmboxManager;

class FilmboxTask : public IAsyncTask<int>
{
public:
    FilmboxTask(const std::string &path, FilmboxManager *filmboxManager);

private:
    const std::string m_path;

    std::unique_ptr<FbxScene> m_scene;

    std::vector<std::unique_ptr<GameObject>> m_objectlist;

    std::vector<GameObject *> m_objectroot;

    FilmboxManager *m_filmboxManager;

    int doInBackground() override;

    void onPostExecute(int result) override;

    void load_object(const FbxScene *scene);

    GameObject *create_from(const FbxNode &node, Transform *parent = nullptr);

    void generateMesh(const FbxNode *node, GameObject *go);
};

/*
#include "gr-base/core/filmbox/fbxScene.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <atomic>

class FilmboxTask
{
public:
    FilmboxTask(const std::string &path, std::function<void ()> func = nullptr);
    ~FilmboxTask();

    void add_function(std::function<void ()> func);

    void render();

    void save();

private:
    std::vector<std::unique_ptr<GameObject>> m_objectlist;

    std::vector<GameObject *> m_objectroot;

    std::vector<std::function<void ()>> funclist;

    std::unique_ptr<FbxScene> m_scene;

    std::string m_name;

    std::atomic<bool> bcanrender;

    void render_hierarchy(GameObject *go);

    void invoke();

    void doInBackground();

    void load_object(const FbxScene *scene);

    GameObject *create_from(const FbxNode &node, Transform *parent = nullptr);

    void generateMesh(const FbxNode *node, GameObject *go);
};
*/



