/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "kineticmodel.h"

#include <QTimer>
#include <QDateTime>

static const int KineticModelDefaultUpdateInterval = 15; // ms

class KineticModelPrivate
{
public:
    QTimer ticker;

    bool released;
    qreal position;
    qreal velocity;
    qreal maximumSpeed;
    qreal deacceleration;

    QTime timestamp;
    qreal lastPosition;

    KineticModelPrivate();
};

KineticModelPrivate::KineticModelPrivate()
    : released(false)
    , position(0)
    , velocity(0)
    , maximumSpeed(1500)
    , deacceleration(900)
    , lastPosition(0)
{

}

KineticModel::KineticModel(QObject *parent)
    : QObject(parent)
    , d_ptr(new KineticModelPrivate)
{
    connect(&d_ptr->ticker, SIGNAL(timeout()), SLOT(update()));
    d_ptr->ticker.setInterval(KineticModelDefaultUpdateInterval);
}

KineticModel::~KineticModel()
{

}

qreal KineticModel::position() const
{
    return d_ptr->position;
}

void KineticModel::setPosition(qreal pos)
{
    d_ptr->released = false;

    qreal oldPos = d_ptr->position;
    d_ptr->position = pos;
    if (!qFuzzyCompare(pos, oldPos))
        emit positionChanged(pos);

    update();
    if (!d_ptr->ticker.isActive())
        d_ptr->ticker.start();
}

qreal KineticModel::maximumSpeed() const
{
    return d_ptr->maximumSpeed;
}

void KineticModel::setMaximumSpeed(qreal maxSpeed)
{
    if (maxSpeed <= 0)
        return;
    d_ptr->maximumSpeed = maxSpeed;
}

qreal KineticModel::deacceleration() const
{
    return d_ptr->deacceleration;
}

void KineticModel::setDeacceleration(qreal deacceleration)
{
    d_ptr->deacceleration = deacceleration;
}

int KineticModel::updateInterval() const
{
    return d_ptr->ticker.interval();
}

void KineticModel::setUpdateInterval(int ms)
{
    d_ptr->ticker.setInterval(ms);
}

void KineticModel::resetSpeed()
{
    d_ptr->velocity = 0;
    d_ptr->lastPosition = d_ptr->position;
}

void KineticModel::release()
{
    update();
    d_ptr->released = true;

    d_ptr->velocity = qBound(-d_ptr->maximumSpeed, d_ptr->velocity, d_ptr->maximumSpeed);

    if (!d_ptr->ticker.isActive())
        d_ptr->ticker.start();
}

void KineticModel::update()
{
    Q_D(KineticModel);

    int elapsed = d_ptr->timestamp.elapsed();

    // too fast gives less accuracy
    if (elapsed < 5)
        return;

    qreal delta = static_cast<qreal>(elapsed) / 1000.0;

    if (d->released) {
        d->position += (d->velocity * delta);
        qreal vstep = d->deacceleration * delta;
        if (d->velocity < vstep && d->velocity >= -vstep) {
            d->velocity = 0;
            d->ticker.stop();
        } else {
            if (d->velocity > 0)
                d->velocity -= vstep;
            else
                d->velocity += vstep;
        }
        emit positionChanged(d->position);
    } else {
        qreal lastSpeed = d->velocity;
        qreal currentSpeed = (d->position - d->lastPosition) / delta;
        d->velocity = .75 * lastSpeed + .25 * currentSpeed;
        d->lastPosition = d->position;
    }

    d->timestamp.start();
}
