#include "Project_SDL1.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>

//*****************************************************************************
// ********************************* OBJECT ***********************************
//*****************************************************************************
object::object()
{}
/////////////////////////////////////////////
bool object::hasValue(std::string pKey)
{
    return this->values_.count(pKey);
}
/////////////////////////////////////////////
int object::getValue(std::string pKey)
{
    return this->values_[pKey];
}
/////////////////////////////////////////////
void object::setValue(std::string pKey, int pValue)
{
    this->values_[pKey] = pValue;
}
/////////////////////////////////////////////
void object::addPropertie(std::string pPropertie)
{
    this->properties_.push_back(pPropertie);
}
/////////////////////////////////////////////
bool object::hasPropertie(std::string pPropertie)
{
    return (std::find(this->properties_.begin(), this->properties_.end(), pPropertie) != this->properties_.end());
}
/////////////////////////////////////////////
bool object::removePropertie(std::string pPropertie)
{
    std::vector<std::string>::iterator itr = std::find(this->properties_.begin(), this->properties_.end(), pPropertie);
    if (itr != this->properties_.end())
    {
        this->properties_.erase(itr);
        return true;
    }
    return false;
}
//*****************************************************************************
// ***************************** RENDERED OBJECT ******************************
//*****************************************************************************
renderedObject::renderedObject(SDL_Surface* image_ptr, SDL_Surface* window_surface_ptr, int width, int height, int x, int y) :
    object()
{
    this->image_ptr_ = image_ptr;
    this->window_surface_ptr_ = window_surface_ptr;
    this->width_ = width;
    this->height_ = height;
    this->x_ = x;
    this->y_ = y;
}
int renderedObject::getHeightBox() { return this->height_ * 4 / 5; }
int renderedObject::getWidthBox() { return this->width_ / 2; }
int renderedObject::getXBox() { return this->x_ + (this->width_ - this->getWidthBox()) / 2; }
int renderedObject::getYBox() { return this->y_ + (this->height_ - this->getHeightBox()) / 2; }
int renderedObject::getX() { return this->x_; }
int renderedObject::getY() { return this->y_; }

/////////////////////////////////////////////
int renderedObject::getDistance(renderedObject* pO2)
{
    int xDistance = std::min(abs(this->getXBox() - pO2->getXBox()), abs(this->getXBox() - pO2->getXBox() - pO2->getWidthBox()));
    int yDistance = std::min(abs(this->getYBox() - pO2->getYBox()), abs(this->getYBox() - pO2->getYBox() - pO2->getHeightBox()));
    return (int)std::sqrt(xDistance * xDistance + yDistance * yDistance);
}
/////////////////////////////////////////////
bool renderedObject::overlap(renderedObject* vO2)
{
    return!((this->getXBox() > vO2->getXBox() + vO2->getWidthBox())
        || (this->getXBox() + this->getWidthBox() < vO2->getXBox())
        || (this->getYBox() > vO2->getYBox() + vO2->getHeightBox())
        || (this->getYBox() + this->getHeightBox() < vO2->getYBox()));
}
/////////////////////////////////////////////
void renderedObject::draw()
{
    //La position (et pas la taille) de ce rectangle d�finie l'endroit ou la surface est coll�e
    SDL_Rect vRect = { this->x_, this->y_,0, 0 };
    SDL_BlitSurface(this->image_ptr_, NULL, this->window_surface_ptr_, &vRect);
}
//*****************************************************************************
// ****************************** MOVING OBJECT *******************************
//*****************************************************************************
movingObject::movingObject(int totalVelocity)
{
    this->totalVelocity_ = totalVelocity;
    this->setRandomVelocitys();
}
/////////////////////////////////////////////
bool movingObject::canMoveX() { return (this->getXBox() + this->xVelocity_ + this->getWidthBox() < frame_width) && (this->getXBox() + this->xVelocity_ > 0); }
bool movingObject::canMoveY() { return (this->getYBox() + this->yVelocity_ + this->getHeightBox() < frame_height) && (this->getYBox() + this->yVelocity_ > 0); }
/////////////////////////////////////////////
void movingObject::goToward(renderedObject* vMO) { this->goToward(vMO->getXBox(), vMO->getYBox()); }
void movingObject::goToward(int x, int y)
{
    this->xVelocity_ = x - this->getXBox();
    this->yVelocity_ = y - this->getYBox();
    this->adjustVelocitys();
}
/////////////////////////////////////////////
void movingObject::runAway(renderedObject* vMO) { this->runAway(vMO->getXBox(), vMO->getYBox()); }
void movingObject::runAway(int x, int y)
{
    this->xVelocity_ = this->getXBox() - x;
    this->yVelocity_ = this->getYBox() - y;
    this->adjustVelocitys();
}
/////////////////////////////////////////////
void movingObject::setRandomVelocitys()
{
    this->xVelocity_ = (rand() % this->totalVelocity_ * 2) - this->totalVelocity_;
    if (!canMoveX())
        this->xVelocity_ = -this->xVelocity_;
    this->yVelocity_ = (((rand() % 1) * 2) - 1) * (this->totalVelocity_ - abs(this->xVelocity_));
    if (!canMoveY())
        this->yVelocity_ = -this->yVelocity_;
}
/////////////////////////////////////////////
void movingObject::adjustVelocitys()
{
    //Hors map
    while (!canMoveX() && this->xVelocity_ != 0)
        this->xVelocity_ -= (this->xVelocity_ > 0 ? 1 : -1);
    while (!canMoveY() && this->yVelocity_ != 0)
        this->yVelocity_ -= (this->yVelocity_ > 0 ? 1 : -1);
    //Vitesse trop �lev�
    while (abs(this->xVelocity_) + abs(this->yVelocity_) > abs(this->totalVelocity_))
    {
        this->xVelocity_ -= (this->xVelocity_ > 0 ? 1 : -1);
        if (abs(this->xVelocity_) + abs(this->yVelocity_) > abs(this->totalVelocity_))
            this->yVelocity_ -= (this->yVelocity_ > 0 ? 1 : -1);
    }
    //Vitesse trop faible
    if (abs(this->xVelocity_) + abs(this->yVelocity_) < abs(this->totalVelocity_))
        this->setRandomVelocitys();
}
/////////////////////////////////////////////
void movingObject::move()
{
    if (!canMoveX() || !canMoveY())
        this->setRandomVelocitys();
    this->x_ += this->xVelocity_;
    this->y_ += this->yVelocity_;
}
//*****************************************************************************
// ***************************** ANIMATED OBJECT ******************************
//*****************************************************************************
animatedObject::animatedObject(int frameInterval)
{
    
    this->frameInterval_ = frameInterval;
    this->frameDuration_ = frameInterval;
    this->frameIndex_ = 0;
}
/////////////////////////////////////////////
void animatedObject::updateFrameDuration()
{
    this->frameDuration_++;
    if (this->frameDuration_ >= this->frameInterval_)
        this->nextFrame();
}
/////////////////////////////////////////////
void animatedObject::nextFrame()
{
    this->frameIndex_++;
    std::string imageKey = this->getImageKey();
    if (this->frameIndex_ >= this->images_.at(imageKey).size())
        this->frameIndex_ = 0;
    this->image_ptr_ = this->images_.at(imageKey)[this->frameIndex_];
    this->frameDuration_ = 0;
}
/////////////////////////////////////////////
std::map<std::string, std::vector<SDL_Surface*>>
animatedObject::createSurfaceMap(std::map<std::string, std::vector<std::string>> pPathMap)
{
    std::map<std::string, std::vector<SDL_Surface*>> surfaceMap;
    std::map<std::string, std::vector<std::string>>::iterator it;
    for (it = pPathMap.begin(); it != pPathMap.end(); it++)
    {
        std::string vKey = it->first;
        std::vector<std::string> vPaths = it->second;
        std::vector<SDL_Surface*> vSurfaces = {};
        for (std::string vPath : vPaths)
            vSurfaces.push_back(IMG_Load(vPath.c_str()));
        surfaceMap.insert({ vKey,vSurfaces });
    }
    return surfaceMap;
}
//*****************************************************************************
// ********************************* SHEPERD **********************************
//*****************************************************************************
SDL_Surface* shepherd::ShepherdImage  = IMG_Load("media/shepherd.png");
int shepherd::ImgW = 49;
int shepherd::ImgH = 49;
//*****************************************************************************
shepherd::shepherd(SDL_Surface* window_surface_ptr) :
    renderedObject(ShepherdImage, window_surface_ptr, shepherd::ImgW, shepherd::ImgH, frame_width / 2, frame_height / 2), movingObject(4)
{
    this->properties_ = {"shepherd"};
}
/////////////////////////////////////////////
void shepherd::update()
{
    this->move();
    this->draw();
}
/////////////////////////////////////////////
void shepherd::interact(renderedObject* pO2)
{}
/////////////////////////////////////////////
void shepherd::move()
{
    const uint8_t* keystate = SDL_GetKeyboardState(0);
    //Horizontal Velocity
    if (keystate[SDL_SCANCODE_LEFT])  
        this->xVelocity_ = -this->totalVelocity_; 
    else if (keystate[SDL_SCANCODE_RIGHT]) 
        this->xVelocity_ = this->totalVelocity_; 
    else 
        this->xVelocity_ = 0; 
    //Vertical Velocity
    if (keystate[SDL_SCANCODE_UP])  
        this->yVelocity_ = -this->totalVelocity_; 
    else if (keystate[SDL_SCANCODE_DOWN]) 
        this->yVelocity_ = this->totalVelocity_; 
    else 
        this->yVelocity_ = 0; 
    //Move
    if (canMoveX())
        this->x_ += this->xVelocity_;
    if (canMoveY())
        this->y_ += this->yVelocity_;
}
//*****************************************************************************
//*********************************** WOLF ************************************
//*****************************************************************************
std::map<std::string, std::vector<SDL_Surface*>> wolf::WolfImages = wolf::createSurfaceMap(wolf::createPathMap());
SDL_Surface* wolf::WolfImage = IMG_Load("media/wolf.png");
int wolf::ImgW = 157;
int wolf::ImgH = 110;
//*****************************************************************************
wolf::wolf(SDL_Surface* window_surface_ptr, int x, int y) :
    renderedObject(WolfImage, window_surface_ptr, wolf::ImgW, wolf::ImgH, x, y), animatedObject(4), movingObject(4)
{
    this->images_ = WolfImages;
    this->values_.insert({ "preyDistance", -1 });
    this->values_.insert({ "timeBeforeStarve", 400 });
    this->properties_ = { "wolf" };
}
/////////////////////////////////////////////
wolf::wolf(SDL_Surface* window_surface_ptr) :
    wolf::wolf(window_surface_ptr, (rand() % (frame_width - wolf::ImgW)), (rand() % (frame_height - wolf::ImgH))){}
/////////////////////////////////////////////
void wolf::update()
{
    this->move();
    this->updateFrameDuration();
    this->draw();
    this->updateTimeBeforeStarve();
    this->values_["preyDistance"]=-1;
    this->removePropertie("scared");
}
/////////////////////////////////////////////
void wolf::interact(renderedObject* pO2)
{
    if (pO2->hasPropertie("shepherd") && this->getDistance(pO2) < 150)
    {
        this->addPropertie("scared");
        this->runAway(pO2);
    }
    else if (this->overlap(pO2) && pO2->hasPropertie("prey"))
    {
        this->setValue("timeBeforeStarve", 400);
        pO2->addPropertie("dead");
    }
    else if (!this->hasPropertie("scared") && pO2->hasPropertie("prey"))
    {
        int distance = this->getDistance(pO2);
        if (this->values_["preyDistance"] == -1 || this->values_["preyDistance"] > distance)
        {
            this->values_["preyDistance"] = distance;
            this->goToward(pO2);
        }
    }
}
/////////////////////////////////////////////
void wolf::updateTimeBeforeStarve()
{
    this->values_["timeBeforeStarve"]--;  
    if (this->values_["timeBeforeStarve"] <= 0)
        this->addPropertie("dead");
}
/////////////////////////////////////////////
std::map<std::string, std::vector<std::string>> wolf::createPathMap()
{
    std::string p = "media/wolfs/";
    std::vector<std::string> vPNW, vPNE, vPSW, vPSE;
    for (int i = 1; i <= 12; i++)
    {
        vPNW.push_back(p + "nw (" + std::to_string(i) + ").png");
        vPNE.push_back(p + "ne (" + std::to_string(i) + ").png");
        vPSW.push_back(p + "sw (" + std::to_string(i) + ").png");
        vPSE.push_back(p + "se (" + std::to_string(i) + ").png");
    }
    std::map<std::string, std::vector<std::string>> pathMap = { {"nw",vPNW},{"ne",vPNE},{"sw",vPSW},{"se",vPSE} };
    return pathMap;
}
/////////////////////////////////////////////
std::string wolf::getImageKey()
{
    if (this->xVelocity_ <= 0 && this->yVelocity_ >= 0) { return "sw"; }
    if (this->xVelocity_ >= 0 && this->yVelocity_ >= 0) { return "se"; }
    if (this->xVelocity_ <= 0 && this->yVelocity_ <= 0) { return "nw"; }
    return "ne";
}
//*****************************************************************************
//*********************************** SHEEP ***********************************
//*****************************************************************************
std::map<std::string, std::vector<SDL_Surface*>> sheep::SheepImagesM = sheep::createSurfaceMap(sheep::createPathMap("male"));
std::map<std::string, std::vector<SDL_Surface*>> sheep::SheepImagesF = sheep::createSurfaceMap(sheep::createPathMap("female"));
SDL_Surface* sheep::SheepImage = IMG_Load("media/sheep.png");
int sheep::ImgW = 68;
int sheep::ImgH = 60;
//*****************************************************************************
sheep::sheep(SDL_Surface* window_surface_ptr, int x, int y) :
    renderedObject(SheepImage, window_surface_ptr, sheep::ImgW, sheep::ImgH, x, y), animatedObject(10), movingObject(3)
{
    this->values_.insert({"timeBeforeProcreate" , 0});
    this->properties_ = {"sheep", "prey", "canprocreate", (rand() % 2 ? "male" : "female") };
    this->images_ = (this->hasPropertie("female") ? SheepImagesF : SheepImagesM);
}
/////////////////////////////////////////////
sheep::sheep(SDL_Surface* window_surface_ptr) :
    sheep(window_surface_ptr, (rand() % (frame_width - sheep::ImgW)), (rand() % (frame_height - sheep::ImgH)))
{}
/////////////////////////////////////////////
void sheep::update()
{
    this->move();
    this->updateFrameDuration();
    this->draw();
    this->values_["timeBeforeProcreate"]--;
}
/////////////////////////////////////////////
void sheep::interact(renderedObject* pO2)
{
    //Run away a wolf
    if (pO2->hasPropertie("wolf") && this->getDistance(pO2) < 200)
        this->runAway(pO2);
    //Reproduce
    else if (this->hasPropertie("male") && pO2->hasPropertie("female") && this->overlap(pO2)
         && this->getValue("timeBeforeProcreate") <= 0 && pO2->getValue("timeBeforeProcreate") <= 0)
    {
        this->setValue("timeBeforeProcreate", 500);
        pO2->setValue("timeBeforeProcreate", 500);
        pO2->addPropertie("pregnant");
    }
}
/////////////////////////////////////////////
std::map<std::string, std::vector<std::string>> sheep::createPathMap(std::string pGender)
{
    std::string p = (pGender == "female" ? "media/sheepsF/" : "media/sheepsM/");
    std::vector<std::string> vPNW, vPNE, vPSW, vPSE;
    for (int i = 1; i <= 10; i++)
    {
        vPNW.push_back(p + "nw (" + std::to_string(i) + ").png");
        vPNE.push_back(p + "ne (" + std::to_string(i) + ").png");
        vPSW.push_back(p + "sw (" + std::to_string(i) + ").png");
        vPSE.push_back(p + "se (" + std::to_string(i) + ").png");
    }
    std::map<std::string, std::vector<std::string>> pathMap = { {"nw",vPNW},{"ne",vPNE},{"sw",vPSW},{"se",vPSE} };
    return pathMap;
}
/////////////////////////////////////////////
std::string sheep::getImageKey()
{
    if (this->xVelocity_ <= 0 && this->yVelocity_ >= 0) { return "sw"; }
    if (this->xVelocity_ >= 0 && this->yVelocity_ >= 0) { return "se"; }
    if (this->xVelocity_ <= 0 && this->yVelocity_ <= 0) { return "nw"; }
    return "ne";
}
//*****************************************************************************
//******************************** APPLICATION ********************************
//*****************************************************************************
application::application(int n_sheep,int n_wolf)
{
    this->window_ptr_ = SDL_CreateWindow("W", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, frame_width, frame_height, 0);
    this->window_surface_ptr_ = SDL_GetWindowSurface(this->window_ptr_);
    if (this->window_ptr_ == NULL || this->window_surface_ptr_ == NULL) { exit(1); }
    this->setGround(n_sheep, n_wolf);
}
/////////////////////////////////////////////
void application::setGround(int n_sheep, int n_wolf)
{
    this->ground_ = new ground(this->window_surface_ptr_);
    for (int sheepNbr = 0; sheepNbr < n_sheep; sheepNbr++)
        this->ground_->addMovingObject(new sheep(this->window_surface_ptr_));
    for (int wolfNbr = 0; wolfNbr < n_wolf; wolfNbr++)
        this->ground_->addMovingObject(new wolf(this->window_surface_ptr_));
    this->ground_->addMovingObject(new shepherd(this->window_surface_ptr_));
}
/////////////////////////////////////////////
void application::loop(int duration)
{
    SDL_Event e;
    int startTime = SDL_GetTicks();
    //Loop
    while (SDL_GetTicks() - startTime < duration * 1000)
    {
        //Check if cross clicked
        while (SDL_PollEvent(&e)) 
            if (e.type == SDL_QUIT)
                exit(0); 
        //Update screen
        int startTimeUpdate = SDL_GetTicks();
        this->ground_->update();
        int updateDuration = SDL_GetTicks() - startTimeUpdate;
        SDL_UpdateWindowSurface(this->window_ptr_);
        //Wait
        SDL_Delay(std::max(0, 1000 / FPS - updateDuration));
    }
}
//*****************************************************************************
// ********************************** GROUND **********************************
//*****************************************************************************
ground::ground(SDL_Surface* window_surface_ptr)
{
    this->window_surface_ptr_ = window_surface_ptr;
    this->movingObjects_ = {};
}
/////////////////////////////////////////////
void ground::addMovingObject(movingObject* pO)
{
    this->movingObjects_.push_back(pO);
}
/////////////////////////////////////////////
void ground::update()
{
    this->drawGround();
    this->makeInteract();
    this->updateObjects();
    this->removeDeads();
    this->addNews();
}
/////////////////////////////////////////////
void ground::drawGround()
{
    SDL_Rect vRect = { 0,0,frame_width ,frame_height };
    SDL_FillRect(this->window_surface_ptr_, &vRect, 0x04A88D);
}
/////////////////////////////////////////////
void ground::makeInteract()
{
    for (int vO1 = 0; vO1 < this->movingObjects_.size()-1; vO1++)
        for (int vO2 = vO1 + 1; vO2 < this->movingObjects_.size(); vO2++)
        {
            this->movingObjects_[vO1]->interact(this->movingObjects_[vO2]);
            this->movingObjects_[vO2]->interact(this->movingObjects_[vO1]);
        }
}
/////////////////////////////////////////////
void ground::updateObjects()
{
    for(movingObject* vMO: this->movingObjects_)
        vMO->update();
}
/////////////////////////////////////////////
void ground::removeDeads()
{
    std::vector<movingObject*>::iterator it = this->movingObjects_.begin();
    while (it != this->movingObjects_.end())
        if ((*it)->hasPropertie("dead"))
        {
            //delete(*it);
            it = this->movingObjects_.erase(it);
        }
        else
            it++;
}
/////////////////////////////////////////////
void ground::addNews()
{
    int n = this->movingObjects_.size();
    for (int i = 0; i < n - 1; i++)
    {
        movingObject* vMO = this->movingObjects_[i];
        if (vMO->removePropertie("pregnant"))
            this->addMovingObject(new sheep(this->window_surface_ptr_, vMO->getX(), vMO->getY()));
    }
}